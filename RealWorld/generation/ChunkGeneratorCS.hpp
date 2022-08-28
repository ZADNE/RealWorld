/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/vertices/ShaderProgram.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/shaders/generation.hpp>
#include <RealWorld/constants/chunk.hpp>

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
    void selectVariants() override;
    void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) override;


    RE::ShaderProgram m_structureShd = RE::ShaderProgram{ {.comp = structure_comp} };
    RE::ShaderProgram m_consolidationShd = RE::ShaderProgram{ {.comp = consolidation_comp} };
    RE::ShaderProgram m_variantSelectionShd = RE::ShaderProgram{ {.comp = variantSelection_comp} };

    std::array<RE::Texture, 2> m_tilesTex = {
        RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}},
        RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}}
    };
    RE::Texture m_materialGenTex{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE} };
};