/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorCS.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

constexpr int GEN_CS_GROUP_SIZE = 16;

ChunkGeneratorCS::ChunkGeneratorCS() {
    m_generateStructureShd.backInterfaceBlock(0u, UNIF_BUF_GENERATION);
    m_consolidateEdgesShd.backInterfaceBlock(0u, UNIF_BUF_GENERATION);
    m_selectVariantShd.backInterfaceBlock(0u, UNIF_BUF_GENERATION);

    m_tilesTex[0].bind(TEX_UNIT_GEN_TILES[0]);
    m_tilesTex[1].bind(TEX_UNIT_GEN_TILES[1]);
    m_materialGenTex.bind(TEX_UNIT_GEN_MATERIAL);

    m_tilesTex[0].bindImage(IMG_UNIT_GEN_TILES[0], 0, RE::ImageAccess::READ_WRITE);
    m_tilesTex[1].bindImage(IMG_UNIT_GEN_TILES[1], 0, RE::ImageAccess::READ_WRITE);
    m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);
}

void ChunkGeneratorCS::prepareToGenerate() {

}

void ChunkGeneratorCS::generateBasicTerrain() {
    m_generateStructureShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
}

void ChunkGeneratorCS::consolidateEdges() {
    using enum RE::IncoherentAccessBarrierFlags;
    glm::uint cycleN = 0u;
    auto pass = [this, &cycleN, &generationBuf](const glm::ivec2& thresholds, size_t passes) {
        generationBuf.overwrite(offsetof(ChunkGeneratorUniforms, edgeConsolidationThresholds), thresholds);
        for (size_t i = 0; i < passes; i++) {
            generationBuf.overwrite(offsetof(ChunkGeneratorUniforms, edgeConsolidationCycle), cycleN++);
            RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
            m_consolidateEdgesShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, false);
        }
    };
    auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    m_consolidateEdgesShd.use();
    doublePass({3, 4}, {4, 5}, 4);
    m_consolidateEdgesShd.unuse();

    assert(static_cast<int>(cycleN) <= GEN_BORDER_WIDTH);
}

void ChunkGeneratorCS::selectVariant() {
    using enum RE::IncoherentAccessBarrierFlags;
    RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_selectVariantShd.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
}

void ChunkGeneratorCS::finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
    m_tilesTex[0].copyTexels(0, glm::ivec2{GEN_BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
}
