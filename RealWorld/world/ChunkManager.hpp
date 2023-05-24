/*!
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

/**
* @brief Ensures that chunks are activated when needed.
*
* Uploads and downloads chunks from the world texture.
*/
class ChunkManager {
public:

#pragma warning( push )
#pragma warning( disable : 4200 )
    struct ActiveChunksSB {
        glm::ivec2 activeChunksMask;
        glm::ivec2 activeChunksArea;
        glm::ivec4 dynamicsGroupSize;
        glm::ivec2 offsets[];            //First indexes: offsets of update chunks, in tiles
        //Following indexes: absolute positions of chunks, in chunks
    };
#pragma warning( pop )

    /**
     * @brief Contructs chunks manager
     *
     * Chunk manager needs to have set its target to work properly.
     */
    ChunkManager(ChunkGenerator& chunkGen, const RE::PipelineLayout& pipelineLayout);

    /**
     * @brief Retargets the chunk manager to a new world.
     * 
     * @detail                  Frees all chunks of the previous world (does not save them).
     * 
     * @param seed              Seed of the new world.
     * @param folderPath        Path to the folder that contains the new world.
     * @param worldTex          The world texture that will receive the loaded chunks
     * @param activeChunksArea  Size of the main texture that holds active chunks.
     *                          Measured in chunks, must be multiples of 8
     * @return                  Active chunks storage buffer
     */
    const RE::Buffer& setTarget(
        int seed,
        std::string folderPath,
        const RE::Texture& worldTex,
        RE::DescriptorSet& descriptorSet,
        const glm::ivec2& activeChunksArea
    );

    /**
     * @brief Saves all chunks, keeps them in the memory.
     * @note This operation is slow!
     * @return True if successful, false otherwise.
     */
    bool saveChunks();

    /**
     * @brief Gets the number of inactive chunks held in memory.
     *
     * @return The number of chunks held in memory.
     */
    size_t numberOfInactiveChunks();

    void beginStep();

    /**
     * @brief Plans activation of chunks that overlap given rectangular area
     * @param commandBuffer Command buffer that will be used to record the commands
     * @param botLeftTi Bottom left corner of the rectangular area
     * @param topRightTi Top right corner of the rectangular area
     * @warning Must be called between beginStep() and endStep().
    */
    void planActivationOfChunks(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2& botLeftTi,
        const glm::ivec2& topRightTi
    );

    int endStep(const vk::CommandBuffer& commandBuffer);

private:

    void planTransition(const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh);

    void planActivation(
        const vk::CommandBuffer& commandBuffer,
        glm::ivec2& activeChunk,
        const glm::ivec2& posCh,
        const glm::ivec2& posAt
    );

    void planDeactivation(
        const vk::CommandBuffer& commandBuffer,
        glm::ivec2& activeChunk,
        const glm::ivec2& posAt
    );

    bool planUpload(
        const vk::CommandBuffer& commandBuffer,
        const std::vector<unsigned char>& tiles,
        const glm::ivec2& posCh,
        const glm::ivec2& posAt
    );

    bool planDownload(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2& posCh,
        const glm::ivec2& posAt
    );

    void saveChunk(const uint8_t* tiles, glm::ivec2 posCh) const;

    /**
     * @brief Key: posCh, Val: inactive chunk (current tiles)
    */
    std::unordered_map<glm::ivec2, Chunk> m_inactiveChunks;

    static constexpr auto k_chunkNotActive = glm::ivec2{std::numeric_limits<int>::max()};
    static constexpr auto k_chunkBeingDownloaded = k_chunkNotActive - 1;
    static constexpr auto k_chunkBeingUploaded = k_chunkNotActive - 2;

    std::optional<RE::Buffer> m_activeChunksBuf;
    std::optional<RE::Buffer> m_activeChunksStageBuf;
    ActiveChunksSB* m_activeChunksStageMapped = nullptr;
    int m_transparentChunkChanges = 0;       /**< Number of changes in this step */
    glm::ivec2& activeChunkAtIndex(int acIndex) const;

    RE::Pipeline m_analyzeContinuityPl;
    glm::uvec2 m_analyzeContinuityGroupCount{};

    std::string m_folderPath;
    ChunkGenerator& m_chunkGen;
    const RE::Texture* m_worldTex = nullptr;
    glm::ivec2 m_activeChunksMask{};

    //Tile stage
    static constexpr auto k_tileStageSize = 8;
    enum class TileStageTransferState {
        Downloading,
        Uploading
    };
    struct TileStageState {
        TileStageTransferState transfer;
        glm::ivec2 posCh;
    };
    std::array<TileStageState, k_tileStageSize> m_tileStageStates{};
    int m_nextFreeTileStage = 0;
    RE::Buffer m_tilesStageBuf;
    unsigned char* m_tilesStageMapped = m_tilesStageBuf.map<unsigned char>(0ull, VK_WHOLE_SIZE);
};