/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/World.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

#define member(var, m0) offsetof(decltype(var), m0), sizeof(var.m0), &var.m0

namespace rw {

constexpr uint32_t k_worldTexBinding          = 0;
constexpr uint32_t k_activeChunksBufBinding   = 1;
constexpr uint32_t k_tilePropertiesBufBinding = 2;
constexpr uint32_t k_branchBufBinding         = 3;

// Xorshift algorithm by George Marsaglia
uint32_t xorshift32(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

uint32_t permuteOrder(uint32_t& state) {
    uint32_t                permutationIndex = xorshift32(state) % 24;
    uint32_t                order            = 0;
    std::array<uint32_t, 4> offsets{0b00, 0b01, 0b10, 0b11};
    for (uint32_t i = 4; i > 0; i--) {
        uint32_t index = permutationIndex % i;
        order |= offsets[index] << (8 - i * 2);
        std::swap(offsets[index], offsets[i - 1]);
        permutationIndex /= i;
    }
    return order;
}

constexpr static struct TilePropertiesUIB {
    // x = properties
    // yz = indices of first and last transformation rule
    std::array<glm::uvec4, 256> blockTransformationProperties =
        k_blockTransformationProperties;
    std::array<glm::uvec4, 256> wallTransformationProperties =
        k_wallTransformationProperties;

    // x = The properties that neighbors MUST have to transform
    // y = The properties that neighbors MUST NOT have to transform
    // z = Properties of the transformation
    // w = The wall that it will be transformed into
    std::array<glm::uvec4, 16> blockTransformationRules = k_blockTransformationRules;
    std::array<glm::uvec4, 16> wallTransformationRules = k_wallTransformationRules;
} k_tileProperties;

World::World()
    : m_simulationPL(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{k_worldTexBinding, eStorageImage, 1, eCompute},
                    {k_activeChunksBufBinding, eStorageBuffer, 1, eCompute},
                    {k_tilePropertiesBufBinding, eUniformBuffer, 1, eCompute},
                    {k_branchBufBinding, eStorageBuffer, 1, eCompute}}},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(WorldDynamicsPC)}}}
      )
    , m_tilePropertiesBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(TilePropertiesUIB),
          .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
          .initData    = re::objectToByteSpan(k_tileProperties),
          .debugName   = "rw::World::tileProperties"})
    , m_worldDynamicsPC{.timeHash = static_cast<uint32_t>(time(nullptr))} {
    m_simulationDS.write(
        eUniformBuffer, k_tilePropertiesBufBinding, 0, m_tilePropertiesBuf, 0, vk::WholeSize
    );
}

const re::Texture& World::adoptSave(const MetadataSave& save, glm::ivec2 worldTexSizeCh) {
    m_seed      = save.seed;
    m_worldName = save.worldName;

    // Resize the world texture
    glm::uvec2 texSize = chToTi(worldTexSizeCh);
    using enum vk::ImageUsageFlagBits;
    m_worldTex = re::Texture{re::TextureCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eDedicatedMemory,
        .format     = vk::Format::eR8G8B8A8Uint,
        .extent     = {texSize, 1},
        .usage      = eStorage | eTransferSrc | eTransferDst | eSampled |
                 eColorAttachment | eInputAttachment,
        .debugName = "rw::World::world"}};
    m_simulationDS.write(eStorageImage, k_worldTexBinding, 0, m_worldTex, eGeneral);

    // Body simulator
    const auto& bodiesBuf = m_bodySimulator.adoptSave(worldTexSizeCh);

    // Vegetation simulator
    auto vegStorage = m_vegSimulator.adoptSave(m_worldTex, worldTexSizeCh);
    m_simulationDS.write(
        eStorageBuffer, k_branchBufBinding, 0, vegStorage.branchBuf, 0, vk::WholeSize
    );

    // Update chunk manager
    m_activeChunksBuf = &m_chunkActivationMgr.setTarget(ChunkActivationMgr::TargetInfo{
        .seed          = m_seed,
        .folderPath    = save.path,
        .worldTex      = m_worldTex,
        .worldTexCh    = worldTexSizeCh,
        .descriptorSet = m_simulationDS,
        .bodiesBuf     = bodiesBuf,
        .branchBuf     = vegStorage.branchBuf});

    return m_worldTex;
}

void World::gatherSave(MetadataSave& save) const {
    save.seed      = m_seed;
    save.worldName = m_worldName;
}

bool World::saveChunks() {
    // Unrasterize branches so that the saved chunks do not contain them
    re::CommandBuffer::doOneTimeSubmit([&](const re::CommandBuffer& cmdBuf) {
        m_vegSimulator.unrasterizeVegetation(cmdBuf);
    });

    // Save the chunks
    return m_chunkActivationMgr.saveChunks();
}

size_t World::numberOfInactiveChunks() {
    return m_chunkActivationMgr.numberOfInactiveChunks();
}

void World::step(
    const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftTi, glm::ivec2 topRightTi
) {
    // Unrasterize branches
    m_vegSimulator.unrasterizeVegetation(cmdBuf);

    // Activation manager
    cmdBuf->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_simulationPL, 0, *m_simulationDS, {}
    );
    m_chunkActivationMgr.activateArea(
        cmdBuf, botLeftTi, topRightTi, m_vegSimulator.writeBuf()
    );

    // Bodies
    // m_bodySimulator.step(cmdBuf);

    // Rasterize branches
    m_vegSimulator.rasterizeVegetation(cmdBuf);

    // Tile transformations
    tileTransformationsStep(cmdBuf);

    // Fluid dynamics
    fluidDynamicsStep(cmdBuf, botLeftTi, topRightTi);
}

void World::modify(
    const re::CommandBuffer& cmdBuf,
    TileLayer                layer,
    ModificationShape        shape,
    float                    radius,
    glm::ivec2               posTi,
    glm::uvec2               tile
) {
    m_worldDynamicsPC.globalPosTi    = posTi;
    m_worldDynamicsPC.modifyTarget   = static_cast<glm::uint>(layer);
    m_worldDynamicsPC.modifyShape    = static_cast<glm::uint>(shape);
    m_worldDynamicsPC.modifyRadius   = radius;
    m_worldDynamicsPC.modifySetValue = tile;
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_modifyTilesPl);
    cmdBuf->pushConstants<WorldDynamicsPC>(
        *m_simulationPL, eCompute, 0, m_worldDynamicsPC
    );
    cmdBuf->dispatch(1, 1, 1);
    auto imageBarrier = re::imageMemoryBarrier(
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eComputeShader,                              // Dst stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        m_worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

void World::prepareWorldForDrawing(const re::CommandBuffer& cmdBuf) {
    // Transit world texture back to readonly-optimal layout so that it can rendered
    auto imageBarrier = re::imageMemoryBarrier(
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eComputeShader | S::eVertexShader,           // Dst stage mask
        A::eShaderSampledRead,                          // Dst access mask
        eGeneral,                                       // Old image layout
        eReadOnlyOptimal,                               // New image layout
        m_worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

void World::fluidDynamicsStep(
    const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftTi, glm::ivec2 topRightTi
) {
    auto dbg = cmdBuf.createDebugRegion("fluid dynamics");
    // Convert positions to chunks
    glm::ivec2 botLeftCh    = tiToCh(botLeftTi);
    glm::ivec2 topRightCh   = tiToCh(topRightTi);
    glm::ivec2 dispatchSize = topRightCh - botLeftCh;

    // Wait on tile transformations
    auto imageBarrier = re::imageMemoryBarrier(
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eComputeShader,                              // Dst stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        m_worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    // Permute the orders
    uint32_t order;
    if (m_permuteOrder) {
        order                         = 0;
        m_worldDynamicsPC.updateOrder = 0;
        // 4 random orders, the threads randomly select from these
        for (unsigned int i = 0; i < 4; i++) {
            m_worldDynamicsPC.updateOrder |=
                permuteOrder(m_worldDynamicsPC.timeHash) << (i * 8);
        }
        cmdBuf->pushConstants(
            *m_simulationPL, eCompute, member(m_worldDynamicsPC, updateOrder)
        );
        // Randomize order of dispatches
        order = permuteOrder(m_worldDynamicsPC.timeHash);
    } else {
        order = 0b00011011;
    }

    // 4 rounds, each updates one quarter of the chunks
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateFluidsPl);
    glm::ivec2 dynBotLeftTi = botLeftCh * iChunkTi + iChunkTi / 2;
    for (unsigned int i = 0; i < 4; i++) {
        // Update offset of the groups
        glm::ivec2 offset{(order >> (i * 2 + 1)) & 1, (order >> (i * 2)) & 1};
        m_worldDynamicsPC.globalPosTi = dynBotLeftTi + offset * iChunkTi / 2;
        cmdBuf->pushConstants(
            *m_simulationPL, eCompute, member(m_worldDynamicsPC, globalPosTi)
        );
        // Dispatch
        cmdBuf->dispatch(dispatchSize.x, dispatchSize.y, 1);
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }
}

void World::tileTransformationsStep(const re::CommandBuffer& cmdBuf) {
    // Set up cmdBuf state for simulation
    auto dbg = cmdBuf.createDebugRegion("tile transformations");
    xorshift32(m_worldDynamicsPC.timeHash);
    cmdBuf->pushConstants(
        *m_simulationPL, eCompute, member(m_worldDynamicsPC, timeHash)
    );

    // Barrier from branch rasterization
    auto imageBarrier = re::imageMemoryBarrier(
        S::eColorAttachmentOutput,                          // Src stage mask
        A::eColorAttachmentRead | A::eColorAttachmentWrite, // Src access mask
        S::eComputeShader,                                  // Dst stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite,     // Dst access mask
        eGeneral,                                           // Old image layout
        eGeneral,                                           // New image layout
        m_worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    // Tile transformations
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_transformTilesPl);
    cmdBuf->dispatchIndirect(
        **m_activeChunksBuf, offsetof(ActiveChunksSB, dynamicsGroupSize)
    );
}

} // namespace rw
