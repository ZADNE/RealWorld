/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/vertices/ShaderProgram.hpp>
#include <RealEngine/rendering/output/Surface.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/shaders/generation.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/constants/chunk.hpp>

/**
 * @brief Generates new chunks by switching framebuffers.
*/
class ChunkGeneratorFBO : public ChunkGenerator {
public:

    ChunkGeneratorFBO();

private:

    void prepareToGenerate() override;
    void generateBasicTerrain() override;
    void consolidateEdges() override;
    void selectVariants() override;
    void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) override;

    RE::VertexArray m_VAO;
    RE::ShaderProgram m_structureShd = RE::ShaderProgram{ {.vert = chunkGen_vert, .frag = structure_frag} };
    RE::ShaderProgram m_consolidationShd = RE::ShaderProgram{ {.vert = chunkGen_vert, .frag = consolidation_frag} };
    RE::ShaderProgram m_variantSelectionShd = RE::ShaderProgram{ {.vert = chunkGen_vert, .frag = variationSelection_frag} };

    std::array<RE::Surface, 2> m_genSurf = {
        RE::Surface{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 2, true, false},
        RE::Surface{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 1, true, false}
    };
};