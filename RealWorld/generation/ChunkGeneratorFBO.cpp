/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorFBO.hpp>

#include <RealEngine/rendering/Ordering.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>


template<RE::Renderer R>
ChunkGeneratorFBO<R>::ChunkGeneratorFBO() {
    m_structureShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);
    m_consolidationShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);
    m_variantSelectionShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);

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
    m_structureShd.use();
    m_va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
    m_structureShd.unuse();
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::consolidateEdges() {
    unsigned int cycleN = 0u;
    auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
        m_consolidationShd.setUniform(LOC_THRESHOLDS, thresholds);
        for (size_t i = 0; i < passes; i++) {
            m_consolidationShd.setUniform(LOC_CYCLE_N, cycleN++);
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

    m_consolidationShd.use();
    doublePass({3, 4}, {4, 5}, 4);
    m_consolidationShd.unuse();

    m_genSurf[0].setTarget();
    outputs.push_back(RE::FramebufferOutput::TO_COLOR1);
    m_genSurf[0].associateTexturesWithOutputs(outputs);
    assert(static_cast<int>(cycleN) <= GEN_BORDER_WIDTH);
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::selectVariants() {
    RE::Ordering<R>::issueDrawBarrier();
    m_variantSelectionShd.use();
    m_va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
    m_variantSelectionShd.unuse();
}

template<RE::Renderer R>
void ChunkGeneratorFBO<R>::finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) {
    m_genSurf[0].resetTarget();
    m_va.unbind();
    m_genSurf[0].getTexture().copyTexels(0, glm::ivec2{GEN_BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
}

template ChunkGeneratorFBO<RE::RendererGL46>;
