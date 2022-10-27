/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

template<RE::Renderer R>
ChunkGenerator<R>::ChunkGenerator() {

}

template<RE::Renderer R>
void ChunkGenerator<R>::setSeed(int seed) {
    m_seed = seed;
    m_chunkUniformBuf.overwrite(offsetof(ChunkUniforms, seed), sizeof(seed), &seed);
}

template<RE::Renderer R>
void ChunkGenerator<R>::generateChunk(const glm::ivec2& posCh, const RE::Texture<R>& destinationTexture, const glm::ivec2& destinationOffset) {
    //Update chunk offset within the uniform buffer
    glm::ivec2 offsetTi = posCh * iCHUNK_SIZE;
    m_chunkUniformBuf.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(offsetTi), &offsetTi);

    prepareToGenerate();

    //Actual generation
    generateBasicTerrain();
    consolidateEdges();
    selectVariant();

    finishGeneration(destinationTexture, destinationOffset);
}

template class ChunkGenerator<RE::RendererVK13>;
template class ChunkGenerator<RE::RendererGL46>;
