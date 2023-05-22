/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/CommandBuffer.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>
#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>

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
    void finishGeneration(const vk::CommandBuffer& commandBuffer, const RE::Texture& dstTex, const glm::ivec2& dstOffset) override;

    vk::ImageMemoryBarrier2 stepBarrier() const;

    RE::PipelineLayout m_pipelineLayout{
        {},
        {.comp = generateStructure_comp}
    };
    RE::Pipeline m_generateStructurePl{
        {.pipelineLayout = *m_pipelineLayout},
        {.comp = generateStructure_comp}
    };
    RE::Pipeline m_consolidateEdgesPl{
        {.pipelineLayout = *m_pipelineLayout},
        {.comp = consolidateEdges_comp}
    };
    RE::Pipeline m_selectVariantPl{
        {.pipelineLayout = *m_pipelineLayout},
        {.comp = selectVariant_comp}
    };
    RE::DescriptorSet m_descSet{m_pipelineLayout, 0u};

    RE::Texture m_tilesTex{RE::TextureCreateInfo{
        .format = vk::Format::eR8G8B8A8Uint,
        .extent = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .layers = 2,
        .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
        .initialLayout = vk::ImageLayout::eGeneral
    }};
    RE::Texture m_materialTex{RE::TextureCreateInfo{
        .format = vk::Format::eR8G8B8A8Uint,
        .extent = {k_genChunkSize.x, k_genChunkSize.y, 1u},
        .usage = vk::ImageUsageFlagBits::eStorage,
        .initialLayout = vk::ImageLayout::eGeneral
    }};
};