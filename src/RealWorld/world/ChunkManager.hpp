/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/commands/CommandBuffer.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

namespace rw {

class ActivationManager;

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
    ChunkManager(const re::PipelineLayout& pipelineLayout, ActivationManager& actManager);

    /**
     * @brief   Retargets the chunk manager to a new world.
     * @detail  Frees all chunks of the previous world without saving them.
     * @param targetInfo    Info about the new world
     */
    void setTarget(glm::ivec2 worldTexSizeCh);

    /**
     * @brief Saves all chunks, keeps them in the memory.
     * @note This operation is slow!
     * @return True if successful, false otherwise.
     */
    bool saveChunks(const re::Texture& worldTex);

    /**
     * @brief Finishes tile transfers from previous step
     * @return Number of finished uploads (= number of transparent changes)
     */
    int beginStep();

    /**
     * @brief Uploads and downloads can only be planned when there is enough space
     */
    [[nodiscard]] bool hasFreeTransferSpace() const;

    /**
     * @brief Plans an upload transfer
     * @pre  hasFreeTransferSpace() == true
     * @note Must be called between beginStep() and endStep()
     */
    void planUpload(
        const std::vector<unsigned char>& tiles, glm::ivec2 posCh, glm::ivec2 posAt
    );

    /**
     * @brief Plans a download transfer
     * @pre  hasFreeTransferSpace() == true
     * @note Must be called between beginStep() and endStep()
     */
    void planDownload(glm::ivec2 posCh, glm::ivec2 posAt);

    /**
     * @brief Peforms all previously planned transfers
     */
    void endStep(const re::CommandBuffer& cmdBuf, const re::Texture& worldTex);

private:
    glm::ivec2 m_worldTexSizeMask{};

    ActivationManager& m_actManager;

    // Tile stage
    static constexpr auto k_tileStageSize = 16;
    /**
     * @brief Target global position of the transfered chunk, in chunks
     */
    std::array<glm::ivec2, k_tileStageSize> m_tileStageTargetCh{};
    /**
     * @brief Uploads are emplaced at the beginning, downloads at the end
     */
    std::array<vk::BufferImageCopy2, k_tileStageSize> m_copyRegions;
    re::BufferMapped<unsigned char>                   m_tilesStageBuf;

    int  m_nextUploadTileStage   = 0;
    int  m_nextDownloadTileStage = k_tileStageSize - 1;
    void resetTileStages();
};

} // namespace rw
