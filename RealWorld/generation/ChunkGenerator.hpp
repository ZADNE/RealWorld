/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/Pipeline.hpp>

#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/constants/generation.hpp>

 /**
  * @brief Is an interface for chunk generators.
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
     * @brief Generates a chunk. The pixels are stored inside given texture at given position.
     *
     * @param posCh Position of the chunk (measured in chunks)
     * @param destinationTexture The texture that will receive the generated chunk
     * @param destinationOffset Offset within destinationTexture where the texels/tiles will be copied
     */
    void generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset);

protected:

    struct PushConstants {
        glm::ivec2 chunkOffsetTi;
        int seed;
        glm::uint edgeConsolidationCycle;
        glm::ivec2 edgeConsolidationThresholds;
    };

    PushConstants m_pushConstants;

    virtual void prepareToGenerate() = 0;
    virtual void generateBasicTerrain() = 0;
    virtual void consolidateEdges() = 0;
    virtual void selectVariant() = 0;
    virtual void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) = 0;
};