/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/commands/CommandBuffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/constants/generation.hpp>
#include <RealWorld/generation/shaders/AllShaders.hpp>
#include <RealWorld/generation/shaders/GenerationPC.glsl.hpp>
#include <RealWorld/simulation/general/ActionCmdBuf.hpp>

namespace rw {

/**
 * @brief Generates new chunks
 * @details The class is implemented in multiple files:
 *          ChunkGeneratorMain.cpp, ChunkGeneratorTerrain.cpp and
 *          ChunkGeneratorVeg.cpp.
 */
class ChunkGenerator {
public:
    /**
     * @brief Needs to have its target set before it can be used
     */
    ChunkGenerator();

    struct TargetInfo {
        int seed;                    ///< Controls how generated chunks look
        const re::Texture& worldTex; ///< Receives the generated tiles
        glm::ivec2 worldTexSizeCh;
        const re::Buffer& bodiesBuf; ///< Receives the generated bodies
        const re::Buffer& branchBuf;
        const re::Buffer& branchAllocRegBuf;
    };

    /**
     * @brief Sets where to put the generated data
     */
    void setTarget(const TargetInfo& targetInfo);

    /**
     * @brief Plans generation a chunk - tiles and vegetation
     * @return True if it could be planned
     */
    [[nodiscard]] bool planGeneration(glm::ivec2 posCh);

    /**
     * @brief Generates planned chunks
     * @return Number of chunk generated
     */
    int generate(const ActionCmdBuf& acb);

private:
    void generateBasicTerrain(const re::CommandBuffer& cb);
    void consolidateEdges(const re::CommandBuffer& cb);
    void selectVariant(const re::CommandBuffer& cb);

    void generateVegetation(const ActionCmdBuf& acb);

    void copyToDestination(const ActionCmdBuf& acb);

    const re::Texture* m_worldTex = nullptr;
    const re::Buffer* m_bodiesBuf = nullptr;
    const re::Buffer* m_branchBuf = nullptr;

    glsl::GenerationPC m_genPC{.storeSegment = 0};
    static_assert(sizeof(glsl::GenerationPC) <= 128, "PC min size guarantee");
    int m_chunksPlanned = 0; ///< Number of chunks planned to generate this step

    re::StepDoubleBuffered<re::CommandBuffer> m_cb{
        re::CommandBuffer{
            {.level     = vk::CommandBufferLevel::eSecondary,
             .debugName = "rw::ChunkGenerator::cb[0]"}
        },
        re::CommandBuffer{
            {.level     = vk::CommandBufferLevel::eSecondary,
             .debugName = "rw::ChunkGenerator::cb[1]"}
        }
    };

    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::ChunkGenerator::descriptorSet"
    }};

    // Tile generation
    re::Pipeline m_generateStructurePl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::generateStructure"},
        {.comp = glsl::generateStructure_comp}
    };
    re::Pipeline m_consolidateEdgesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::consolidateEdges"},
        {.comp = glsl::consolidateEdges_comp}
    };
    re::Pipeline m_selectVariantPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::selectVariant"},
        {.comp = glsl::selectVariant_comp}
    };
    re::Texture m_layerTex{re::TextureCreateInfo{
        .format = vk::Format::eR16G16Uint,
        .extent = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .layers = k_chunkGenSlots * 3, // 2 segments for blocks, 1 for walls
        .usage  = vk::ImageUsageFlagBits::eStorage |
                 vk::ImageUsageFlagBits::eTransferSrc,
        .initialLayout = vk::ImageLayout::eGeneral,
        .debugName     = "rw::ChunkGenerator::layer"
    }};
    re::Texture m_materialTex{re::TextureCreateInfo{
        .format        = vk::Format::eR16Uint,
        .extent        = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .layers        = k_chunkGenSlots,
        .usage         = vk::ImageUsageFlagBits::eStorage,
        .initialLayout = vk::ImageLayout::eGeneral,
        .debugName     = "rw::ChunkGenerator::material"
    }};

    // Vegetation generation
    re::Pipeline m_selectVegSpeciesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::selectVegSpecies"},
        {.comp = glsl::selectVegSpecies_comp}
    };
    re::Pipeline m_expandVegInstancesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::expandVegInstances"},
        {.comp = glsl::expandVegInstances_comp}
    };
    re::Pipeline m_allocBranchesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::allocBranches"},
        {.comp = glsl::allocBranches_comp}
    };
    re::Pipeline m_outputBranchesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::outputBranches"},
        {.comp = glsl::outputBranches_comp}
    };
    re::Buffer m_vegTemplatesBuf;
    re::Buffer m_vegPrepBuf;
};

} // namespace rw
