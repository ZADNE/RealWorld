/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/rendering/vertices/ShaderProgram.hpp>
#include <RealEngine/rendering/output/Surface.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>

#include <RealWorld/shaders/generation.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/constants/chunk.hpp>

/**
 * @brief Is an interface for chunk generators.
*/
class ChunkGenerator {
public:

    /**
     * @brief Width of the padding around the generated chunk.
     *
     * Avoids errors at the edges of generated chunks.
    */
    const int BORDER_WIDTH = 8;

    /**
     * @brief Size of the area that is generated for each chunk
     *
     * It is bigger than the actual chunk because it contains the padding around.
     *
     * @see BORDER_WIDTH
    */
    const glm::ivec2 GEN_CHUNK_SIZE = iCHUNK_SIZE + 2 * BORDER_WIDTH;

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

    using enum RE::BufferType;
    using enum RE::BufferStorage;
    using enum RE::BufferAccessFrequency;
    using enum RE::BufferAccessNature;
    using enum RE::BufferUsageFlags;
    using enum RE::VertexComponentCount;
    using enum RE::VertexComponentType;
    using enum RE::Primitive;
    struct ChunkUniforms {
        glm::ivec2 chunkOffsetTi;
        int seed;
    };
    RE::TypedBuffer m_chunkUniformBuf{UNIF_BUF_CHUNKGEN, sizeof(ChunkUniforms), RE::BufferUsageFlags::DYNAMIC_STORAGE};

    virtual void prepareToGenerate() = 0;
    virtual void generateBasicTerrain() = 0;
    virtual void consolidateEdges() = 0;
    virtual void selectVariants() = 0;
    virtual void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) = 0;

    int m_seed = 0;
};