/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

#include <RealEngine/rendering/CommandBuffer.hpp>


ChunkGenerator::ChunkGenerator() {

}

void ChunkGenerator::setSeed(int seed) {
    m_pushConstants.seed = seed;
}

void ChunkGenerator::generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
    m_pushConstants.chunkOffsetTi = posCh * iCHUNK_SIZE;

    prepareToGenerate();

    //Actual generation
    generateBasicTerrain();
    consolidateEdges();
    selectVariant();

    finishGeneration(destinationTexture, destinationOffset);
}
