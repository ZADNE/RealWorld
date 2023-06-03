/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/graphics/CommandBuffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/shaders/AllShaders.hpp>

/**
 * @brief Generates new chunks by compute shaders.
 */
class ChunkGeneratorCS: public ChunkGenerator {
public:
    ChunkGeneratorCS();

private:
    void prepareToGenerate(const vk::CommandBuffer& commandBuffer) override;
    void generateBasicTerrain(const vk::CommandBuffer& commandBuffer) override;
    void consolidateEdges(const vk::CommandBuffer& commandBuffer) override;
    void selectVariant(const vk::CommandBuffer& commandBuffer) override;
    void finishGeneration(
        const vk::CommandBuffer& commandBuffer,
        const re::Texture&       dstTex,
        const glm::ivec2&        dstOffset
    ) override;

    vk::ImageMemoryBarrier2 stepBarrier() const;

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