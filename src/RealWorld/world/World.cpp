/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/World.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

#define member(var, m0) offsetof(decltype(var), m0), sizeof(var.m0), &var.m0

namespace rw {

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

struct TilePropertiesUIB {
    // x = properties
    // yz = indices of first and last transformation rule
    std::array<glm::uvec4, 256> blockTransformationProperties;
    std::array<glm::uvec4, 256> wallTransformationProperties;

    // x = The properties that neighbors MUST have to transform
    // y = The properties that neighbors MUST NOT have to transform
    // z = Properties of the transformation
    // w = The wall that it will be transformed into
    std::array<glm::uvec4, 16> blockTransformationRules;
    std::array<glm::uvec4, 16> wallTransformationRules;
};

static constexpr TilePropertiesUIB k_tileProperties = TilePropertiesUIB{
    .blockTransformationProperties = k_blockTransformationProperties,
    .wallTransformationProperties  = k_wallTransformationProperties,
    .blockTransformationRules      = k_blockTransformationRules,
    .wallTransformationRules       = k_wallTransformationRules};

World::World()
    : m_tilePropertiesBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(TilePropertiesUIB),
          .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
          .initData    = re::objectToByteSpan(k_tileProperties)})
    , m_worldDynamicsPC{.timeHash = static_cast<uint32_t>(time(nullptr))} {
    m_simulationDS.write(eUniformBuffer, 2, 0, m_tilePropertiesBuf, 0, vk::WholeSize);
}

const re::Texture& World::adoptSave(
    const MetadataSave& save, const glm::ivec2& worldTexSizeCh
) {
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
                 eColorAttachment | eInputAttachment}};
    m_simulationDS.write(eStorageImage, 0, 0, m_worldTex, eGeneral);

    // Body simulator
    const auto& bodiesBuf = m_bodySimulator.adoptSave(worldTexSizeCh);
    m_simulationDS.write(eStorageBuffer, 3, 0, bodiesBuf, 0, vk::WholeSize);

    // Vegetation simulator
    auto vegStorage = m_vegSimulator.adoptSave(m_worldTex, worldTexSizeCh);

    // Update chunk manager
    m_activeChunksBuf = &m_chunkManager.setTarget(ChunkManager::TargetInfo{
        .seed           = m_seed,
        .folderPath     = save.path,
        .worldTex       = m_worldTex,
        .worldTexSizeCh = worldTexSizeCh,
        .descriptorSet  = m_simulationDS,
        .bodiesBuf      = bodiesBuf,
        .vegBuf         = vegStorage.vegBuf,
        .branchBuf      = vegStorage.branchBuf});

    return m_worldTex;
}

void World::gatherSave(MetadataSave& save) const {
    save.seed      = m_seed;
    save.worldName = m_worldName;
}

bool World::saveChunks() {
    return m_chunkManager.saveChunks();
}

size_t World::numberOfInactiveChunks() {
    return m_chunkManager.numberOfInactiveChunks();
}

void World::beginStep(const vk::CommandBuffer& cmdBuf) {
    // Transit world texture to general layout so that compute shaders can
    // manipulate it
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader | S::eVertexShader, // Src stage mask
        A::eShaderSampledRead | A::eShaderStorageRead |
            A::eShaderStorageWrite,        // Src access mask
        S::eComputeShader,                 // Dst stage mask
        A::eShaderStorageRead,             // Dst access mask
        vk::ImageLayout::eReadOnlyOptimal, // Old image layout
        vk::ImageLayout::eGeneral,         // New image layout
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        m_worldTex.image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

int World::step(
    const vk::CommandBuffer& cmdBuf,
    const glm::ivec2&        botLeftTi,
    const glm::ivec2&        topRightTi
) {
    // Chunk manager
    m_chunkManager.beginStep();
    m_chunkManager.planActivationOfChunks(
        cmdBuf, botLeftTi, topRightTi, m_vegSimulator.writeBuf()
    );
    cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_simulationPL, 0, *m_simulationDS, {}
    );
    int activatedChunks = m_chunkManager.endStep(cmdBuf);

    // Bodies
    m_bodySimulator.step(cmdBuf);

    // Vegetation
    m_vegSimulator.step(cmdBuf);

    // Set up cmdBuf state for simulation
    xorshift32(m_worldDynamicsPC.timeHash);
    cmdBuf.pushConstants(*m_simulationPL, eCompute, member(m_worldDynamicsPC, timeHash));

    // Tile transformations
    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, *m_transformTilesPl);
    cmdBuf.dispatchIndirect(
        **m_activeChunksBuf,
        offsetof(ChunkManager::ActiveChunksSB, dynamicsGroupSize)
    );

    // Fluid dynamics
    fluidDynamicsStep(cmdBuf, botLeftTi, topRightTi);

    return activatedChunks;
}

void World::modify(
    const vk::CommandBuffer& cmdBuf,
    TileLayer                layer,
    ModificationShape        shape,
    float                    radius,
    const glm::ivec2&        posTi,
    const glm::uvec2&        tile
) {
    m_worldDynamicsPC.globalPosTi    = posTi;
    m_worldDynamicsPC.modifyTarget   = static_cast<glm::uint>(layer);
    m_worldDynamicsPC.modifyShape    = static_cast<glm::uint>(shape);
    m_worldDynamicsPC.modifyRadius   = radius;
    m_worldDynamicsPC.modifySetValue = tile;
    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, *m_modifyTilesPl);
    cmdBuf.pushConstants<WorldDynamicsPC>(*m_simulationPL, eCompute, 0, m_worldDynamicsPC);
    cmdBuf.dispatch(1, 1, 1);
}

void World::endStep(const vk::CommandBuffer& cmdBuf) {
    // Transit world texture back to readonly-optimal layout so that it can rendered
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eVertexShader,                               // Dst stage mask
        A::eShaderSampledRead,                          // Dst access mask
        eGeneral,                                       // Old image layout
        eReadOnlyOptimal,                               // New image layout
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        m_worldTex.image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

void World::fluidDynamicsStep(
    const vk::CommandBuffer& cmdBuf,
    const glm::ivec2&        botLeftTi,
    const glm::ivec2&        topRightTi
) {
    // Convert positions to chunks
    glm::ivec2 botLeftCh    = tiToCh(botLeftTi);
    glm::ivec2 topRightCh   = tiToCh(topRightTi);
    glm::ivec2 dispatchSize = topRightCh - botLeftCh;

    // Wait on tile transformations
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eComputeShader,                              // Dst stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        m_worldTex.image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

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
        cmdBuf.pushConstants(
            *m_simulationPL, eCompute, member(m_worldDynamicsPC, updateOrder)
        );
        // Randomize order of dispatches
        order = permuteOrder(m_worldDynamicsPC.timeHash);
    } else {
        order = 0b00011011;
    }

    // 4 rounds, each updates one quarter of the chunks
    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateFluidsPl);
    glm::ivec2 dynBotLeftTi = botLeftCh * iChunkTi + iChunkTi / 2;
    for (unsigned int i = 0; i < 4; i++) {
        // Update offset of the groups
        glm::ivec2 offset{(order >> (i * 2 + 1)) & 1, (order >> (i * 2)) & 1};
        m_worldDynamicsPC.globalPosTi = dynBotLeftTi + offset * iChunkTi / 2;
        cmdBuf.pushConstants(
            *m_simulationPL, eCompute, member(m_worldDynamicsPC, globalPosTi)
        );
        // Dispatch
        cmdBuf.dispatch(dispatchSize.x, dispatchSize.y, 1);
        cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }
}

} // namespace rw
