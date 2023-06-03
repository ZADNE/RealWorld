/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/graphics/CommandBuffer.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>

void ChunkGenerator::setSeed(int seed) {
    m_pushConstants.seed = seed;
}

void ChunkGenerator::generateChunk(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2&        posCh,
    const re::Texture&       destinationTexture,
    const glm::ivec2&        destinationOffset
) {
    m_pushConstants.chunkOffsetTi = posCh * iChunkTi;

    prepareToGenerate(commandBuffer);

    // Actual generation
    generateBasicTerrain(commandBuffer);
    consolidateEdges(commandBuffer);
    selectVariant(commandBuffer);

    finishGeneration(commandBuffer, destinationTexture, destinationOffset);
}
