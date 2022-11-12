﻿/*!
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
template<RE::Renderer R>
class ChunkGeneratorCS : public ChunkGenerator<R> {
public:

    ChunkGeneratorCS();

private:

    void prepareToGenerate() override;
    void generateBasicTerrain() override;
    void consolidateEdges() override;
    void selectVariant() override;
    void finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) override;


    RE::ShaderProgram<R> m_generateStructureShd{{.comp = generateStructure_comp}};
    RE::ShaderProgram<R> m_consolidateEdgesShd{{.comp = consolidateEdges_comp}};
    RE::ShaderProgram<R> m_selectVariantShd{{.comp = selectVariant_comp}};

    std::array<RE::Texture<R>, 2> m_tilesTex = {
        RE::Texture<R>{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}},
        RE::Texture<R>{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}}
    };
    RE::Texture<R> m_materialGenTex{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE} };
};