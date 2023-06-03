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
    uint32_t                 permutationIndex = xorshift32(state) % 24;
    uint32_t                 order            = 0u;
    std::array<uint32_t, 4u> offsets{0b00, 0b01, 0b10, 0b11};
    for (uint32_t i = 4u; i > 0u; i--) {
        uint32_t index = permutationIndex % i;
        order |= offsets[index] << (8u - i * 2u);
        std::swap(offsets[index], offsets[i - 1u]);
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

World::World(ChunkGenerator& chunkGen)
    : m_chunkManager(chunkGen, m_pipelineLayout)
    , m_tilePropertiesBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(TilePropertiesUIB),
          .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
          .initData    = &k_tileProperties})
    , m_worldDynamicsPC{.timeHash = static_cast<uint32_t>(time(nullptr))} {
    m_descriptorSet.write(
        eUniformBuffer, 2u, 0u, m_tilePropertiesBuf, 0ull, VK_WHOLE_SIZE
    );
}

const re::Texture& World::adoptSave(
    const MetadataSave& save, const glm::ivec2& activeChunksArea
) {
    m_seed      = save.seed;
    m_worldName = save.worldName;

    // Resize the world texture
    glm::uvec2 texSize = iChunkTi * activeChunksArea;
    using enum vk::ImageUsageFlagBits;
    m_worldTex = re::Texture{re::TextureCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eDedicatedMemory,
        .format     = vk::Format::eR8G8B8A8Uint,
        .extent     = {texSize, 1u},
        .usage      = eStorage | eTransferSrc | eTransferDst | eSampled}};
    m_descriptorSet.write(eStorageImage, 0u, 0u, *m_worldTex, eGeneral);

    m_activeChunksBuf = &m_chunkManager.setTarget(
        m_seed, save.path, *m_worldTex, m_descriptorSet, activeChunksArea
    );

    return *m_worldTex;
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

void World::beginStep(const vk::CommandBuffer& commandBuffer) {
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
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

int World::step(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2&        botLeftTi,
    const glm::ivec2&        topRightTi
) {
    // Chunk manager
    m_chunkManager.beginStep();
    m_chunkManager.planActivationOfChunks(commandBuffer, botLeftTi, topRightTi);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    int activatedChunks = m_chunkManager.endStep(commandBuffer);

    // Set up commandBuffer state for simulation
    xorshift32(m_worldDynamicsPC.timeHash);
    commandBuffer.pushConstants(
        *m_pipelineLayout, eCompute, member(m_worldDynamicsPC, timeHash)
    );

    // Tile transformations
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_transformTilesPl);
    commandBuffer.dispatchIndirect(
        **m_activeChunksBuf,
        offsetof(ChunkManager::ActiveChunksSB, dynamicsGroupSize)
    );

    // Fluid dynamics
    fluidDynamicsStep(commandBuffer, botLeftTi, topRightTi);

    return activatedChunks;
}

void World::modify(
    const vk::CommandBuffer& commandBuffer,
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
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_modifyTilesPl);
    commandBuffer.pushConstants<WorldDynamicsPC>(
        *m_pipelineLayout, eCompute, 0u, m_worldDynamicsPC
    );
    commandBuffer.dispatch(1u, 1u, 1u);
}

void World::endStep(const vk::CommandBuffer& commandBuffer) {
    // Transit world texture back to readonly-optimal layout so that it can
    // rendered
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
        S::eVertexShader,                               // Dst stage mask
        A::eShaderSampledRead,                          // Dst access mask
        eGeneral,                                       // Old image layout
        eReadOnlyOptimal,                               // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

void World::fluidDynamicsStep(
    const vk::CommandBuffer& commandBuffer,
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
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    // Permute the orders
    uint32_t order;
    if (m_permuteOrder) {
        order                         = 0u;
        m_worldDynamicsPC.updateOrder = 0u;
        // 4 random orders, the threads randomly select from these
        for (unsigned int i = 0u; i < 4u; i++) {
            m_worldDynamicsPC.updateOrder |=
                permuteOrder(m_worldDynamicsPC.timeHash) << (i * 8u);
        }
        commandBuffer.pushConstants(
            *m_pipelineLayout, eCompute, member(m_worldDynamicsPC, updateOrder)
        );
        // Randomize order of dispatches
        order = permuteOrder(m_worldDynamicsPC.timeHash);
    } else {
        order = 0b00011011;
    }

    // 4 rounds, each updates one quarter of the chunks
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateFluidsPl);
    glm::ivec2 dynBotLeftTi = botLeftCh * iChunkTi + iChunkTi / 2;
    for (unsigned int i = 0u; i < 4u; i++) {
        // Update offset of the groups
        glm::ivec2 offset{(order >> (i * 2u + 1u)) & 1, (order >> (i * 2u)) & 1};
        m_worldDynamicsPC.globalPosTi = dynBotLeftTi + offset * iChunkTi / 2;
        commandBuffer.pushConstants(
            *m_pipelineLayout, eCompute, member(m_worldDynamicsPC, globalPosTi)
        );
        // Dispatch
        commandBuffer.dispatch(dispatchSize.x, dispatchSize.y, 1u);
        commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }
}

} // namespace rw