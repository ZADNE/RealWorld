/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorFBO.hpp>

#include <RealEngine/rendering/Ordering.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>
#include <RealWorld/shaders/generation.hpp>

template<RE::Renderer R>
ChunkGeneratorFBO<R>::ChunkGeneratorFBO() {
    m_generateStructureShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);
    m_consolidateEdgesShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);
    m_selectVariantShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);

    m_genSurf[0].getTexture(0).bind(TEX_UNIT_GEN_TILES[0]);
    m_genSurf[1].getTexture(0).bind(TEX_UNIT_GEN_TILES[1]);
    m_genSurf[0].getTexture(1).bind(TEX_UNIT_GEN_MATERIAL);
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::prepareToGenerate() {
    m_va.bind();
    m_genSurf[0].setTarget();
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::generateBasicTerrain() {
    m_generateStructureShd.use();
    m_va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
    m_generateStructureShd.unuse();
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::consolidateEdges() {
    unsigned int cycleN = 0u;
    auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
        m_consolidateEdgesShd.setUniform(LOC_THRESHOLDS, thresholds);
        for (size_t i = 0; i < passes; i++) {
            m_consolidateEdgesShd.setUniform(LOC_CYCLE_N, cycleN++);
            m_genSurf[(cycleN + 1) % m_genSurf.size()].setTarget();
            RE::Ordering<R>::issueDrawBarrier();
            m_va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
        }
    };
    auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    std::vector<RE::FramebufferOutput> outputs;
    outputs.reserve(2);
    outputs.push_back(RE::FramebufferOutput::TO_COLOR0);

    m_genSurf[0].associateTexturesWithOutputs(outputs);

    m_consolidateEdgesShd.use();
    doublePass({3, 4}, {4, 5}, 4);
    m_consolidateEdgesShd.unuse();

    m_genSurf[0].setTarget();
    outputs.push_back(RE::FramebufferOutput::TO_COLOR1);
    m_genSurf[0].associateTexturesWithOutputs(outputs);
    assert(static_cast<int>(cycleN) <= GEN_BORDER_WIDTH);
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::selectVariant() {
    RE::Ordering<R>::issueDrawBarrier();
    m_selectVariantShd.use();
    m_va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
    m_selectVariantShd.unuse();
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) {
    m_genSurf[0].resetTarget();
    m_va.unbind();
    m_genSurf[0].getTexture().copyTexels(0, glm::ivec2{GEN_BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
}

template class ChunkGeneratorFBO<RE::RendererVK13>;
template class ChunkGeneratorFBO<RE::RendererGL46>;
