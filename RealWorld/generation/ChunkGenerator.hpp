/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/generation/TerrainGenerator.hpp>
#include <RealWorld/generation/TreeGenerator.hpp>

namespace rw {

/**
 * @brief Generates new chunks
 * @details The logic is divided into more classes: TerrainGenerator and TreeGenerator
 */
class ChunkGenerator {
public:
    ChunkGenerator();

    /**
     * @brief Sets the seed that controls how the generated chunks look
     *
     * @param seed Seed of the world
     */
    void setSeed(int seed);

    /**
     * @brief Generates a chunk. The pixels are stored inside given texture at
     * given position.
     * @param commandBuffer Command buffer that is used for the generation
     * @param posCh Position of the chunk (measured in chunks)
     * @param dstTex The texture that will receive the generated chunk
     * @param dstOffset Offset within dstTex where the texels/tiles will be copied to
     */
    void generateChunk(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2&        posCh,
        const re::Texture&       dstTex,
        const glm::ivec2&        dstOffset
    );

protected:
    GenerationPC     m_genPC;
    TerrainGenerator m_terrainGen;
    TreeGenerator    m_treeGen;
};

} // namespace rw