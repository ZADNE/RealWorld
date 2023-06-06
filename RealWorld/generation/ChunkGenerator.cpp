/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

namespace rw {

ChunkGenerator::ChunkGenerator()
    : m_terrainGen(m_genPC)
    , m_treeGen(m_genPC) {
}

void ChunkGenerator::setSeed(int seed) {
    m_genPC.seed = seed;
}

void ChunkGenerator::generateChunk(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2&        posCh,
    const re::Texture&       destinationTexture,
    const glm::ivec2&        destinationOffset
) {
    m_genPC.chunkOffsetTi = posCh * iChunkTi;

    m_terrainGen.prepareToGenerate(commandBuffer);

    // Actual generation
    m_terrainGen.generateBasicTerrain(commandBuffer);
    m_terrainGen.consolidateEdges(commandBuffer);
    m_terrainGen.selectVariant(commandBuffer);

    m_terrainGen.finishGeneration(commandBuffer, destinationTexture, destinationOffset);
}

} // namespace rw
