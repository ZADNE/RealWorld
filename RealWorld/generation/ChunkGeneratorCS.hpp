/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/textures/Texture.hpp>

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
    void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) override;


    RE::Pipeline m_generateStructureShd{{.comp = generateStructure_comp}};
    RE::Pipeline m_consolidateEdgesShd{{.comp = consolidateEdges_comp}};
    RE::Pipeline m_selectVariantShd{{.comp = selectVariant_comp}};

    std::array<RE::Texture, 2> m_tilesTex = {
        RE::Texture{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}},
        RE::Texture{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}}
    };
    RE::Texture m_materialGenTex{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}};
};