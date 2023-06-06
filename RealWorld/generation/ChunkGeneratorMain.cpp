/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

namespace rw {

ChunkGenerator::ChunkGenerator() {
    m_descSet.write(
        vk::DescriptorType::eStorageImage, 0u, 0u, m_tilesTex, vk::ImageLayout::eGeneral
    );
    m_descSet.write(
        vk::DescriptorType::eStorageImage, 1u, 0u, m_materialTex, vk::ImageLayout::eGeneral
    );
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

    prepareToGenerate(commandBuffer);

    // Terrain generation
    generateBasicTerrain(commandBuffer);
    consolidateEdges(commandBuffer);
    selectVariant(commandBuffer);

    finishGeneration(commandBuffer, destinationTexture, destinationOffset);
}

} // namespace rw
