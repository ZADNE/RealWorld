/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

ChunkGenerator::ChunkGenerator() {

}

void ChunkGenerator::setSeed(int seed) {
    m_seed = seed;
    m_chunkUniformBuf.overwrite(offsetof(ChunkUniforms, seed), sizeof(seed), &seed);
}

void ChunkGenerator::generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
    //Update chunk offset within the uniform buffer
    glm::ivec2 offsetTi = posCh * iCHUNK_SIZE;
    m_chunkUniformBuf.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(offsetTi), &offsetTi);

    prepareToGenerate();

    //Actual generation
    generateBasicTerrain();
    consolidateEdges();
    selectVariants();

    finishGeneration(destinationTexture, destinationOffset);
}
