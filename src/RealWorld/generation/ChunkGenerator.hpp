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
#include <RealWorld/vegetation/Branch.hpp>

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
        int                seed;     /**< Controls how generated chunks look */
        const re::Texture& worldTex; /**< Receives the generated tiles */
        glm::ivec2         worldTexSizeCh;
        const re::Buffer&  bodiesBuf; /**< Receives the generated bodies */
        const re::Buffer&  vegBuf;
        const re::Buffer&  branchBuf;
    };

    /**
     * @brief Sets where to put the generated data
     */
    void setTarget(const TargetInfo& targetInfo);

    /**
     * @brief Should be called at the beginning of each step where chunks will
     * be generated
     */
    void nextStep();

    struct OutputInfo {
        glm::ivec2 posCh;          /**< Position of the chunk */
        glm::uint  branchWriteBuf; /**< Index of the double buffered part of
                                     branch buffer that is for writing */
    };

    /**
     * @brief Generates a chunk - tiles and vegetation
     */
    void generateChunk(const re::CommandBuffer& cmdBuf, const OutputInfo& outputInfo);

protected:
    void prepareToGenerate(const re::CommandBuffer& cmdBuf);

    void generateBasicTerrain(const re::CommandBuffer& cmdBuf);
    void consolidateEdges(const re::CommandBuffer& cmdBuf);
    void selectVariant(const re::CommandBuffer& cmdBuf);

    void generateVegetation(const re::CommandBuffer& cmdBuf);

    void finishGeneration(const re::CommandBuffer& cmdBuf, glm::ivec2 posCh);

    static re::Buffer createVegTemplatesBuffer();

    int m_nOfGenChunksThisStep = 0;

    const re::Texture* m_worldTex = nullptr;
    glm::ivec2         m_worldTexSizeCh{};
    const re::Buffer*  m_bodiesBuf = nullptr;
    const re::Buffer*  m_vegBuf    = nullptr;
    const re::Buffer*  m_branchBuf = nullptr;

    struct GenerationPC {
        glm::ivec2 chunkOffsetTi;
        int        seed;
        glm::uint  storeLayer;
        glm::uint  edgeConsolidationPromote;
        glm::uint  edgeConsolidationReduce;
        glm::uint  branchWriteBuf;
    } m_genPC;

    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet  m_descriptorSet{re::DescriptorSetCreateInfo{
         .layout    = m_pipelineLayout.descriptorSetLayout(0),
         .debugName = "rw::ChunkGenerator::descriptorSet"}};

    re::Pipeline m_generateStructurePl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::generateStructure"},
        {.comp = generateStructure_comp}};
    re::Pipeline m_consolidateEdgesPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::consolidateEdges"},
        {.comp = consolidateEdges_comp}};
    re::Pipeline m_selectVariantPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::selectVariant"},
        {.comp = selectVariant_comp}};
    re::Pipeline m_generateVegPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::generateVeg"},
        {.comp = generateVeg_comp}};
    re::Pipeline m_generateVectorVegPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::generateVectorVeg"},
        {.comp = generateVectorVeg_comp}};
    re::Pipeline m_generateRasterVegPl{
        {.pipelineLayout = *m_pipelineLayout,
         .debugName      = "rw::ChunkGenerator::generateRasterVeg"},
        {.comp = generateRasterVeg_comp}};

    re::Texture m_tilesTex{re::TextureCreateInfo{
        .format = vk::Format::eR8G8B8A8Uint,
        .extent = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .layers = 2,
        .usage  = vk::ImageUsageFlagBits::eStorage |
                 vk::ImageUsageFlagBits::eTransferSrc,
        .initialLayout = vk::ImageLayout::eGeneral,
        .debugName     = "rw::ChunkGenerator::tiles"}};
    re::Texture m_materialTex{re::TextureCreateInfo{
        .format        = vk::Format::eR8G8B8A8Uint,
        .extent        = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .usage         = vk::ImageUsageFlagBits::eStorage,
        .initialLayout = vk::ImageLayout::eGeneral,
        .debugName     = "rw::ChunkGenerator::material"}};

    re::Buffer m_vegTemplatesBuf = createVegTemplatesBuffer();
    re::Buffer m_vegPreparationBuf;
};

} // namespace rw
