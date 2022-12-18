/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/CommandBuffer.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>
#include <RealEngine/rendering/DescriptorSet.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/shaders/AllShaders.hpp>

 /**
  * @brief Generates new chunks by compute shaders.
 */
class ChunkGeneratorCS : public ChunkGenerator {
public:

    ChunkGeneratorCS();

private:

    void prepareToGenerate() override;
    void generateBasicTerrain() override;
    void consolidateEdges() override;
    void selectVariant() override;
    void finishGeneration(const RE::Texture& dstTex, const glm::ivec2& dstOffset) override;

    RE::CommandBuffer m_commandBuffer{vk::CommandBufferLevel::ePrimary};
    RE::Pipeline m_generateStructurePl{generateStructure_comp};
    RE::Pipeline m_consolidateEdgesPl{consolidateEdges_comp};
    RE::Pipeline m_selectVariantPl{selectVariant_comp};

    std::array<RE::Texture, 2> m_tilesTex = {
        RE::Texture{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}},
        RE::Texture{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}}
    };
    RE::Texture m_materialGenTex{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}};
};