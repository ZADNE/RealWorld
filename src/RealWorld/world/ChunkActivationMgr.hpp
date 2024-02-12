/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <unordered_map>

#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/world/ActiveChunksSB.hpp>
#include <RealWorld/world/Chunk.hpp>
#include <RealWorld/world/ChunkTransferMgr.hpp>

namespace rw {

/**
 * @brief Ensures correct (de)activation of chunks (tiles and branches)
 */
class ChunkActivationMgr {
public:
    /**
     * @brief Contructs activation manager
     * @note Activation manager needs to have set its target to work properly.
     */
    explicit ChunkActivationMgr(const re::PipelineLayout& pipelineLayout);

    struct TargetInfo {
        int seed;                      /**< Seed of the new world */
        const std::string& folderPath; /**< Path to the folder that contains the new world */
        const re::Texture& worldTex; /**< The world texture that will be managed */
        glm::ivec2         worldTexCh; /**< Must be a multiple of 16 */
        re::DescriptorSet& descriptorSet;
        const re::Buffer&  bodiesBuf;
        const re::Buffer&  branchBuf;
    };

    /**
     * @brief   Retargets the chunk manager to a new world.
     * @detail  Frees all chunks of the previous world without saving them.
     * @param targetInfo    Info about the new world
     * @return              Active chunks storage buffer
     */
    const re::Buffer& setTarget(const TargetInfo& targetInfo);

    /**
     * @brief Saves all chunks, keeps them in the memory.
     * @note This operation is slow!
     * @return True if successful, false otherwise.
     */
    bool saveChunks();

    /**
     * @brief Gets the number of inactive chunks held in memory.
     *
     * @return The number of chunks held in CPU memory.
     */
    size_t numberOfInactiveChunks();

    /**
     * @brief Activates chunks that overlap given rectangular area
     * @param cmdBuf Command buffer that will record the commands
     * @param botLeftTi Bottom left corner of the rectangular area
     * @param topRightTi Top right corner of the rectangular area
     * @param branchWriteBuf Index of the double buffered part of branch buffer
     *                      that is for writing
     */
    void activateArea(
        const re::CommandBuffer& cmdBuf,
        glm::ivec2               botLeftTi,
        glm::ivec2               topRightTi,
        glm::uint                branchWriteBuf
    );

    glm::ivec2& activeChunkAtIndex(int acIndex);

    void addInactiveChunk(glm::ivec2 posCh, Chunk&& chunk);

    void saveChunk(
        glm::ivec2 posCh, const uint8_t* tiles, std::span<const uint8_t> branchesSerialized
    ) const;

private:
    void planTransition(glm::ivec2 posCh, glm::uint branchWriteBuf);

    void planActivation(
        glm::ivec2& activeChunk, glm::ivec2 posCh, glm::ivec2 posAc, glm::uint branchWriteBuf
    );

    void planDeactivation(
        glm::ivec2& activeChunk, glm::ivec2 posAc, glm::uint branchWriteBuf
    );

    void analyzeAfterChanges(const re::CommandBuffer& cmdBuf);

    int m_transparentChunkChanges = 0; /**< Number of changes in this step */

    /**
     * @brief Key: posCh, Val: inactive chunk
     */
    std::unordered_map<glm::ivec2, Chunk> m_inactiveChunks;

    re::Pipeline m_analyzeContinuityPl;
    glm::uvec2   m_analyzeContinuityGroupCount{};

    std::string        m_folderPath;
    const re::Texture* m_worldTex  = nullptr;
    const re::Buffer*  m_branchBuf = nullptr;

    glm::ivec2 m_worldTexMaskCh{};

    re::Buffer                       m_activeChunksBuf;
    re::BufferMapped<ActiveChunksSB> m_activeChunksStageBuf;

    ChunkGenerator   m_chunkGen;
    ChunkTransferMgr m_chunkTransferMgr;
};

} // namespace rw
