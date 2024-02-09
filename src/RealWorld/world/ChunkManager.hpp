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
    explicit ChunkManager(ActivationManager& actManager);

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

    static constexpr auto k_tileStageSlots = 16;
    struct TileStage {
        /**
         * @brief Target global position of the transfered chunk, in chunks
         */
        std::array<glm::ivec2, k_tileStageSlots> targetCh{};
        /**
         * @brief Uploads are emplaced at the beginning, downloads at the end
         */
        std::array<vk::BufferImageCopy2, k_tileStageSlots> copyRegions;
        /**
         * @brief Vulkan coppies from and to this buffer
         */
        re::BufferMapped<unsigned char> buf{re::BufferCreateInfo{
            .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                          vma::AllocationCreateFlagBits::eHostAccessRandom,
            .sizeInBytes = k_tileStageSlots * k_chunkByteSize,
            .usage       = vk::BufferUsageFlagBits::eTransferSrc |
                     vk::BufferUsageFlagBits::eTransferDst,
            .debugName = "rw::ChunkManager::tilesStage"}};

        int  nextUploadSlot   = 0;
        int  nextDownloadSlot = k_tileStageSlots - 1;
        void resetTileStage();
        bool hasFreeTransferSpace() const;
    };
    re::StepDoubleBuffered<TileStage> m_ts;
};

} // namespace rw
