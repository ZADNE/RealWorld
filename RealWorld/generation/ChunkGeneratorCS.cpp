﻿/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorCS.hpp>

#include <RealEngine/rendering/Ordering.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

const int GEN_CS_GROUP_SIZE = 16;

template<RE::Renderer R>
ChunkGeneratorCS<R>::ChunkGeneratorCS() {
    m_structureShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);
    m_variantSelectionShd.backInterfaceBlock(0u, UNIF_BUF_CHUNKGEN);

    m_tilesTex[0].bind(TEX_UNIT_GEN_TILES[0]);
    m_tilesTex[1].bind(TEX_UNIT_GEN_TILES[1]);
    m_materialGenTex.bind(TEX_UNIT_GEN_MATERIAL);

    m_tilesTex[0].bindImage(IMG_UNIT_GEN_TILES[0], 0, RE::ImageAccess::READ_WRITE);
    m_tilesTex[1].bindImage(IMG_UNIT_GEN_TILES[1], 0, RE::ImageAccess::READ_WRITE);
    m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);
}

template<RE::Renderer R>
void ChunkGeneratorCS<R>::prepareToGenerate() {

}

template<RE::Renderer R>
void ChunkGeneratorCS<R>::generateBasicTerrain() {
    m_structureShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
}

template<RE::Renderer R>
void ChunkGeneratorCS<R>::consolidateEdges() {
    using enum RE::IncoherentAccessBarrierFlags;
    unsigned int cycleN = 0u;
    auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
        m_consolidationShd.setUniform(LOC_THRESHOLDS, thresholds);
        for (size_t i = 0; i < passes; i++) {
            m_consolidationShd.setUniform(LOC_CYCLE_N, cycleN++);
            RE::Ordering<R>::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
            m_consolidationShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, false);
        }
    };
    auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    m_consolidationShd.use();
    doublePass({3, 4}, {4, 5}, 4);
    m_consolidationShd.unuse();

    assert(static_cast<int>(cycleN) <= GEN_BORDER_WIDTH);
}

template<RE::Renderer R>
void ChunkGeneratorCS<R>::selectVariants() {
    using enum RE::IncoherentAccessBarrierFlags;
    RE::Ordering<R>::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_variantSelectionShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
}

template<RE::Renderer R>
void ChunkGeneratorCS<R>::finishGeneration(const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) {
    m_tilesTex[0].copyTexels(0, glm::ivec2{GEN_BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
}

template ChunkGeneratorCS<RE::RendererGL46>;
