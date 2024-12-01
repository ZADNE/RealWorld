/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/tiles/World.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

// NOLINTNEXTLINE(*-macro-usage): Very messy but used only here...
#define MEMBER(var, m0) offsetof(decltype(var), m0), sizeof(var.m0), &var.m0

namespace rw {

constexpr glm::uint k_worldTexBinding       = 0;
constexpr glm::uint k_acChunksBinding       = 1;
constexpr glm::uint k_tilePropertiesBinding = 2;
constexpr glm::uint k_branchBinding         = 3;
constexpr glm::uint k_branchAllocRegBinding = 4;
constexpr glm::uint k_branchAllocReqBinding = 5;
constexpr glm::uint k_shaderMessageBinding  = 6;

constexpr float k_stepDurationSec = 1.0f / k_physicsStepsPerSecond;

/// Xorshift algorithm by George Marsaglia
uint32_t xorshift32(uint32_t& state) {
    state ^= state << 13; // NOLINT(*-magic-numbers)
    state ^= state >> 17; // NOLINT(*-magic-numbers)
    state ^= state << 5;  // NOLINT(*-magic-numbers)
    return state;
}

consteval uint32_t factorial(uint32_t n) {
    uint32_t fact = 1;
    for (uint32_t i = 1; i <= n; ++i) fact *= i;
    return fact;
}

uint32_t permuteOrder(uint32_t& state) {
    uint32_t permutationIndex = xorshift32(state) % factorial(4);
    uint32_t order            = 0;
    std::array<uint32_t, 4> offsets{0b00, 0b01, 0b10, 0b11};
    for (uint32_t i = 4; i > 0; i--) {
        uint32_t index = permutationIndex % i;
        order |= offsets[index] << (8 - i * 2);
        std::swap(offsets[index], offsets[i - 1]);
        permutationIndex /= i;
    }
    return order;
}

constexpr static struct TilePropertiesUB {
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
    BlockTransformationRules blockTransformationRules = k_blockTransformationRules;
    WallTransformationRules wallTransformationRules = k_wallTransformationRules;
} k_tileProperties;

World::World(const re::Buffer& shaderMessageBuf)
    : m_simulationPL(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{k_worldTexBinding, eStorageImage, 1, eCompute},
                    {k_acChunksBinding, eStorageBuffer, 1, eCompute},
                    {k_tilePropertiesBinding, eUniformBuffer, 1, eCompute},
                    {k_branchBinding, eStorageBuffer, 1, eCompute},
                    {k_branchAllocRegBinding, eStorageBuffer, 1, eCompute},
                    {k_branchAllocReqBinding, eUniformBuffer, 1, eCompute},
                    {k_shaderMessageBinding, eStorageBuffer, 1, eCompute}}},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(glsl::WorldDynamicsPC)}}
          }
      )
    , m_tilePropertiesBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(TilePropertiesUB),
          .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
          .initData    = re::objectToByteSpan(k_tileProperties),
          .debugName   = "rw::World::tileProperties"
      })
    , m_worldDynamicsPC{.timeHash = static_cast<uint32_t>(time(nullptr))} {
    m_simulationDS.write(eUniformBuffer, k_tilePropertiesBinding, 0, m_tilePropertiesBuf);
    m_simulationDS.write(eStorageBuffer, k_shaderMessageBinding, 0, shaderMessageBuf);
}

const re::Texture& World::adoptSave(
    ActionCmdBuf& acb, const MetadataSave& save, glm::ivec2 worldTexSizeCh
) {
    m_seed      = save.seed;
    m_worldName = save.worldName;

    // Resize the world texture
    glm::ivec2 texSizeTi             = chToTi(worldTexSizeCh);
    m_worldDynamicsPC.worldTexMaskTi = texSizeTi - 1;
    using enum vk::ImageUsageFlagBits;
    m_worldTex = re::Texture{re::TextureCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eDedicatedMemory,
        .format     = vk::Format::eR16G16Uint,
        .extent     = {texSizeTi, 1},
        .layers     = k_tileLayerCount,
        .usage      = eStorage | eTransferSrc | eTransferDst | eSampled |
                 eColorAttachment | eInputAttachment,
        .initialLayout = eShaderReadOnlyOptimal,
        .debugName     = "rw::World::world"
    }};
    m_simulationDS.write(eStorageImage, k_worldTexBinding, 0, m_worldTex, eGeneral);
    acb.track(ImageTrackName::World, m_worldTex, eShaderReadOnlyOptimal, k_tileLayerCount);

    // Body simulator
    const auto& bodiesBuf = m_bodySimulator.adoptSave(worldTexSizeCh);

    // Vegetation simulator
    auto vegStorage = m_vegSimulator.adoptSave(m_worldTex, worldTexSizeCh);
    m_simulationDS.write(eStorageBuffer, k_branchBinding, 0, vegStorage.branchBuf);
    m_simulationDS.write(
        eStorageBuffer, k_branchAllocRegBinding, 0, vegStorage.branchAllocRegBuf
    );
    acb.track(BufferTrackName::Branch, vegStorage.branchBuf);
    acb.track(BufferTrackName::AllocReg, vegStorage.branchAllocRegBuf);

    // Update chunk manager
    auto activationBufs = m_chunkActivationMgr.setTarget(ChunkActivationMgr::TargetInfo{
        .seed              = m_seed,
        .folderPath        = save.path,
        .worldTex          = m_worldTex,
        .worldTexCh        = worldTexSizeCh,
        .descriptorSet     = m_simulationDS,
        .bodiesBuf         = bodiesBuf,
        .branchBuf         = vegStorage.branchBuf,
        .branchAllocRegBuf = vegStorage.branchAllocRegBuf
    });
    acb.track(BufferTrackName::ActiveChunks, activationBufs.activeChunksBuf);

    m_activeChunksBuf = &activationBufs.activeChunksBuf;
    m_simulationDS.write(
        eUniformBuffer, k_branchAllocReqBinding, 0, activationBufs.allocReqBuf
    );

    return m_worldTex;
}

void World::gatherSave(MetadataSave& save) const {
    save.seed      = m_seed;
    save.worldName = m_worldName;
}

bool World::saveChunks(const ActionCmdBuf& acb) {
    // Unrasterize branches so that the saved chunks do not contain them
    re::CommandBuffer::doOneTimeSubmit([&](const re::CommandBuffer& cb) {
        acb.useSecondaryCommandBuffer(cb);
        m_vegSimulator.unrasterizeVegetation(acb);
        acb.stopSecondaryCommandBuffer();
    });

    // Save the chunks
    return m_chunkActivationMgr.saveChunks();
}

size_t World::numberOfInactiveChunks() {
    return m_chunkActivationMgr.numberOfInactiveChunks();
}

void World::step(
    const ActionCmdBuf& acb, glm::ivec2 botLeftTi, glm::ivec2 topRightTi,
    const Hitbox& player
) {
    // Unrasterize branches
    m_vegSimulator.unrasterizeVegetation(acb);

    m_worldDynamicsPC.timeSec += k_stepDurationSec;
    m_worldDynamicsPC.playerPosTi = pxToTi(player.centerPx());

    // Activation manager
    (*acb)->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_simulationPL, 0, *m_simulationDS, {}
    );
    m_chunkActivationMgr.activateArea(acb, botLeftTi, topRightTi);

    // Bodies
    // m_bodySimulator.step(*acb);

    // Rasterize branches
    m_vegSimulator.rasterizeVegetation(acb, m_worldDynamicsPC.timeSec);

    // Tile transformations
    tileTransformationsStep(acb);

    // Fluid dynamics
    fluidDynamicsStep(acb);
}

void World::placeTiles(
    const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
    float radius, glm::ivec2 posTi, glm::uvec2 tile, int maxCount
) {
    modifyTiles(acb, layer, shape, radius, posTi, tile, maxCount);
}

void World::mineTiles(
    const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
    float radius, glm::ivec2 posTi
) {
    modifyTiles(
        acb, layer, shape, radius, posTi, glm::uvec2{0xffffffff, 0},
        std::numeric_limits<int>::max()
    );
}

void World::prepareWorldForDrawing(const ActionCmdBuf& acb) {
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Dummy (is implemented in world drawing classes)
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eComputeShader | S::eFragmentShader,
            .access = A::eShaderSampledRead,
            .layout = eShaderReadOnlyOptimal
        }
    );
}

void World::modifyTiles(
    const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
    float radius, glm::ivec2 posTi, glm::uvec2 tile, int maxCount
) {
    acb.action(
        [&](const re::CommandBuffer& cb) {
            m_worldDynamicsPC.globalOffsetTi = posTi;
            m_worldDynamicsPC.modifyLayer    = std::to_underlying(layer);
            m_worldDynamicsPC.modifyShape    = std::to_underlying(shape);
            m_worldDynamicsPC.modifyRadius   = radius;
            m_worldDynamicsPC.modifySetValue = tile;
            m_worldDynamicsPC.modifyMaxCount = maxCount;
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_modifyTilesPl);
            cb->pushConstants<glsl::WorldDynamicsPC>(
                *m_simulationPL, eCompute, 0, m_worldDynamicsPC
            );
            cb->dispatch(1, 1, 1);
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        BufferAccess{
            .name   = BufferTrackName::ShaderMessage,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        }
    );
}

void World::tileTransformationsStep(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("tile transformations");
    acb.action(
        [&](const re::CommandBuffer& cb) {
            xorshift32(m_worldDynamicsPC.timeHash);
            cb->pushConstants<glsl::WorldDynamicsPC>(
                *m_simulationPL, eCompute, 0, m_worldDynamicsPC
            );
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_transformTilesPl);
            cb->dispatchIndirect(
                **m_activeChunksBuf,
                offsetof(glsl::ActiveChunksSB, dynamicsGroupSize)
            );
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        BufferAccess{
            .name   = BufferTrackName::ActiveChunks,
            .stage  = S::eDrawIndirect,
            .access = A::eIndirectCommandRead
        }
    );
}

void World::fluidDynamicsStep(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("fluid dynamics");
    // Permute the orders
    uint32_t order                = 0;
    m_worldDynamicsPC.updateOrder = 0;
    // 4 random orders, the threads randomly select from these
    for (unsigned int i = 0; i < 4; i++) {
        m_worldDynamicsPC.updateOrder |= permuteOrder(m_worldDynamicsPC.timeHash)
                                         << (i * 8);
    }
    (*acb)->pushConstants(
        *m_simulationPL, eCompute, MEMBER(m_worldDynamicsPC, updateOrder)
    );
    // Randomize order of dispatches
    order = permuteOrder(m_worldDynamicsPC.timeHash);

    // 4 rounds, each updates one quarter of the chunks
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateMovementPl);
    for (unsigned int i = 0; i < 4; i++) {
        acb.action(
            [&](const re::CommandBuffer& cb) {
                // Update offset of the groups
                glm::ivec2 offset{(order >> (i * 2 + 1)) & 1, (order >> (i * 2)) & 1};
                m_worldDynamicsPC.globalOffsetTi = offset * iChunkTi / 2;
                cb->pushConstants(
                    *m_simulationPL, eCompute,
                    MEMBER(m_worldDynamicsPC, globalOffsetTi)
                );
                cb->dispatchIndirect(
                    **m_activeChunksBuf,
                    offsetof(glsl::ActiveChunksSB, dynamicsGroupSize)
                );
            },
            ImageAccess{
                .name   = ImageTrackName::World,
                .stage  = S::eComputeShader,
                .access = A::eShaderStorageRead | A::eShaderStorageWrite
            }
        );
    }
}

} // namespace rw
