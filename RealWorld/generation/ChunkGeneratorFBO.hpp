/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/output/Surface.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/shaders/AllShaders.hpp>

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
    void consolidateEdges(const RE::BufferTyped<R>& generationBuf) override;
    void selectVariant() override;
    void finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) override;

    RE::VertexArray<R> m_va;
    RE::ShaderProgram<R> m_generateStructureShd{{.vert = applyToWholeChunk_vert, .frag = generateStructure_frag}};
    RE::ShaderProgram<R> m_consolidateEdgesShd{{.vert = applyToWholeChunk_vert, .frag = consolidateEdges_frag}};
    RE::ShaderProgram<R> m_selectVariantShd{{.vert = applyToWholeChunk_vert, .frag = selectVariant_frag}};

    std::array<RE::Surface<R>, 2> m_genSurf = {
        RE::Surface<R>{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 2, true},
        RE::Surface<R>{ {GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, 1, true}
    };
};