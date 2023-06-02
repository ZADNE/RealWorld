/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/graphics/textures/Texture.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>

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
     * @param commandBuffer Command buffer that is used for the generation
     * @param posCh Position of the chunk (measured in chunks)
     * @param dstTex The texture that will receive the generated chunk
     * @param dstOffset Offset within dstTex where the texels/tiles will be copied to
     */
    void generateChunk(const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh, const re::Texture& dstTex, const glm::ivec2& dstOffset);

protected:

    struct GenerationPC {
        glm::ivec2 chunkOffsetTi;
        int seed;
        glm::uint storeLayer;
        glm::uint edgeConsolidationPromote;
        glm::uint edgeConsolidationReduce;
    };

    GenerationPC m_pushConstants;

    virtual void prepareToGenerate(const vk::CommandBuffer& commandBuffer) = 0;
    virtual void generateBasicTerrain(const vk::CommandBuffer& commandBuffer) = 0;
    virtual void consolidateEdges(const vk::CommandBuffer& commandBuffer) = 0;
    virtual void selectVariant(const vk::CommandBuffer& commandBuffer) = 0;
    virtual void finishGeneration(const vk::CommandBuffer& commandBuffer, const re::Texture& dstTex, const glm::ivec2& dstOffset) = 0;
};