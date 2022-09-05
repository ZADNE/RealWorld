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

/**
 * @brief Generates new chunks by switching framebuffers.
*/
template<RE::Renderer R>
class ChunkGeneratorFBO : public ChunkGenerator<R> {
public:

    ChunkGeneratorFBO();

private:

    void prepareToGenerate() override;
    void generateBasicTerrain() override;
    void consolidateEdges() override;
    void selectVariants() override;
    void finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) override;

    RE::VertexArray<R> m_va;
    RE::ShaderProgram<R> m_structureShd{{.vert = chunkGen_vert, .frag = structure_frag}};
    RE::ShaderProgram<R> m_consolidationShd{{.vert = chunkGen_vert, .frag = consolidation_frag}};
    RE::ShaderProgram<R> m_variantSelectionShd{{.vert = chunkGen_vert, .frag = variationSelection_frag}};

    std::array<RE::Surface<R>, 2> m_genSurf = {
        RE::Surface<R>{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 2, true},
        RE::Surface<R>{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 1, true}
    };
};