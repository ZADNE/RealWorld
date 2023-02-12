﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <unordered_map>
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>
#include <RealEngine/rendering/buffers/Buffer.hpp>
#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/CommandBuffer.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/world/Chunk.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>


#pragma warning( push )
#pragma warning( disable : 4200 )
struct ActiveChunksSSBO {
    glm::ivec2 activeChunksMask;
    glm::ivec2 activeChunksArea;
    glm::ivec4 dynamicsGroupSize;
    glm::ivec2 offsets[];            //First indexes: offsets of update chunks, in tiles
    //Following indexes: absolute positions of chunks, in chunks
};
#pragma warning( pop )

/**
* @brief Ensures that chunks are activated when needed.
*
* Uploads and downloads chunks from the world texture.
*/
class ChunkManager {
public:

    /**
     * @brief Contructs chunks manager
     *
     * Chunk manager needs to have set its target to work properly.
     */
    ChunkManager(ChunkGenerator& chunkGen);

    /**
     * @brief Retargets the chunk manager to a new world.
     *
     * Frees all chunks of the previous world (does not save them).
     *
     * @param seed Seed of the new world.
     * @param folderPath Path to the folder that contains the new world.
     * @param worldTex The world texture that will receive the loaded chunks
     * @param activeChunksArea Size of the main texture that holds active chunks. Measured in chunks, must be multiples of 8
     */
    void setTarget(int seed, std::string folderPath, RE::Texture& worldTex, const glm::ivec2& activeChunksArea);

    /**
     * @brief Saves all chunks, keeps them in the memory.
     *
     * @return True if successful, false otherwise.
     */
    bool saveChunks() const;

    /**
     * @brief Gets the number of inactive chunks held in memory.
     *
     * @return The number of chunks held in memory.
     */
    size_t getNumberOfInactiveChunks();

    /**
     * @brief Performs beginStep operation on the chunk handler.
     *
     * This function increments internal timers for possible memory deallocation.
     * It should be called every physics beginStep.
    */
    void step();

    /**
     * @brief Forces activation of chunks that overlap given rectangular area
     * @param commandBuffer Command buffer that will be used to record computation commands
     * @param botLeftTi Bottom left corner of the rectangular area
     * @param topRightTi Top right corner of the rectangular area
    */
    int forceActivationOfChunks(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

private:

    /**
     * @brief Activates the chunk at given position.
     * @detail Previous chunk at the position is deactivated.
     * @param commandBuffer Command buffer that will be used for generation
     * @param posCh Position of the chunk
     * @return    1 if the chunk has been activated.
     *            0 if it already has been active and thus it was not activated.
    */
    int activateChunk(const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh);

    /**
     * @brief Deactivates the chunk at given position.
     *
     * Deactivated chunk is placed to inactive chunks storage.
     * Does nothing if there is no active chunk at the position.
     * @param posCh Position of the chunk, measured in chunks
    */
    void deactivateChunk(const glm::ivec2& posCh);

    /**
     * @brief Downloads a chunk from world texture to CPU memory
     * @param posAt In-texture position of the chunk, measured in tiles/pixels
     * @return The tiles of the chunk
    */
    std::vector<unsigned char> downloadChunk(const glm::ivec2& posAt) const;

    /**
     * @brief Uploads tiles from a chunk to the world texture
     * @param chunk Tiles to upload
     * @param posCh Position of the chunk
    */
    void uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const;

    /**
     * @brief Saves tiles from chunk to disk
     * @param chunk Tiles of the chunk. Size must be: iCHUNK_SIZE.x * iCHUNK_SIZE.y * 4 bytes
     * @param posCh Global position of the chunk
    */
    void saveChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const;

    mutable std::unordered_map<glm::ivec2, Chunk> m_inactiveChunks;

    const static inline glm::ivec2 NO_ACTIVE_CHUNK = glm::ivec2(std::numeric_limits<decltype(glm::ivec2::x)>::max());
    std::vector<glm::ivec2> m_activeChunks;

    std::optional<RE::Buffer> m_activeChunksBuf;
    std::optional<RE::Buffer> m_activeChunksStageBuf;
    ActiveChunksSSBO* m_activeChunksStageMapped = nullptr;

    RE::PipelineLayout m_pipelineLayout{{}, {.comp = analyzeContinuity_comp}};
    RE::DescriptorSet m_descriptorSet{m_pipelineLayout, 0u};
    RE::Pipeline m_analyzeContinuityPl{
        {.pipelineLayout = *m_pipelineLayout}, {.comp = analyzeContinuity_comp}
    };
    glm::uvec2 m_analyzeContinuityGroupCount;

    std::string m_folderPath;
    ChunkGenerator& m_chunkGen;
    RE::Texture* m_worldTex = nullptr;
    glm::ivec2 m_activeChunksMask;
    RE::Buffer m_tilesStageBuf;
    unsigned char* m_tilesStageMapped = m_tilesStageBuf.map<unsigned char>(0u, uCHUNK_SIZE.x * uCHUNK_SIZE.y * 4u);
};