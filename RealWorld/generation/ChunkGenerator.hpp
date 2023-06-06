/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/CommandBuffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/constants/generation.hpp>
#include <RealWorld/generation/shaders/AllShaders.hpp>

namespace rw {

/**
 * @brief Generates new chunks
 * @details The class is implemented in multiple files:
 *          ChunkGeneratorMain.cpp, ChunkGeneratorTerrain.cpp and
 *          ChunkGeneratorTree.cpp.
 */
class ChunkGenerator {
public:
    ChunkGenerator();

    /**
     * @brief Sets the seed that controls how the generated chunks look
     *
     * @param seed Seed of the world
     */
    void setSeed(int seed);

    /**
     * @brief Generates a chunk. The pixels are stored inside given texture at
     * given position.
     * @param commandBuffer Command buffer that is used for the generation
     * @param posCh Position of the chunk (measured in chunks)
     * @param dstTex The texture that will receive the generated chunk
     * @param dstOffset Offset within dstTex where the texels/tiles will be copied to
     */
    void generateChunk(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2&        posCh,
        const re::Texture&       dstTex,
        const glm::ivec2&        dstOffset
    );

protected:
    void prepareToGenerate(const vk::CommandBuffer& commandBuffer);
    void generateBasicTerrain(const vk::CommandBuffer& commandBuffer);
    void consolidateEdges(const vk::CommandBuffer& commandBuffer);
    void selectVariant(const vk::CommandBuffer& commandBuffer);
    void finishGeneration(
        const vk::CommandBuffer& commandBuffer,
        const re::Texture&       dstTex,
        const glm::ivec2&        dstOffset
    );

    vk::ImageMemoryBarrier2 stepBarrier() const; /**< Helper func */

    struct GenerationPC {
        glm::ivec2 chunkOffsetTi;
        int        seed;
        glm::uint  storeLayer;
        glm::uint  edgeConsolidationPromote;
        glm::uint  edgeConsolidationReduce;
    };

    GenerationPC m_genPC;

    re::PipelineLayout m_pipelineLayout{{}, {.comp = generateStructure_comp}};
    re::Pipeline       m_generateStructurePl{
              {.pipelineLayout = *m_pipelineLayout}, {.comp = generateStructure_comp}};
    re::Pipeline m_consolidateEdgesPl{
        {.pipelineLayout = *m_pipelineLayout}, {.comp = consolidateEdges_comp}};
    re::Pipeline m_selectVariantPl{
        {.pipelineLayout = *m_pipelineLayout}, {.comp = selectVariant_comp}};
    re::DescriptorSet m_descSet{m_pipelineLayout, 0u};

    re::Texture m_tilesTex{re::TextureCreateInfo{
        .format = vk::Format::eR8G8B8A8Uint,
        .extent = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .layers = 2,
        .usage  = vk::ImageUsageFlagBits::eStorage |
                 vk::ImageUsageFlagBits::eTransferSrc,
        .initialLayout = vk::ImageLayout::eGeneral}};
    re::Texture m_materialTex{re::TextureCreateInfo{
        .format        = vk::Format::eR8G8B8A8Uint,
        .extent        = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .usage         = vk::ImageUsageFlagBits::eStorage,
        .initialLayout = vk::ImageLayout::eGeneral}};
};

} // namespace rw