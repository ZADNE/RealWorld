/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/commands/CommandBuffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/vegetation/BranchSB.hpp>

namespace rw {

class ChunkActivationMgr;

/**
 * @brief Manages uploads and downloads of tiles and branches to/from the world
 * texture and branch buffer
 */
class ChunkTransferMgr {
public:
    explicit ChunkTransferMgr(const re::PipelineLayout& pipelineLayout);

    /**
     * @brief Resets state to no pending transfers
     */
    void reset();

    /**
     * @brief Saves all chunks, keeps them in the memory.
     * @note This operation is slow!
     * @return True if successful, false otherwise.
     */
    bool saveChunks(
        const re::Texture&  worldTex,
        const re::Buffer&   branchBuf,
        glm::ivec2          worldTexCh,
        ChunkActivationMgr& actMgr
    );

    /**
     * @brief Finishes transfers from previous step
     * @return Number of finished uploads (= number of transparent changes)
     */
    int beginStep(glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr);

    /**
     * @brief Uploads and downloads can only be planned when there is enough space
     */
    [[nodiscard]] bool hasFreeTransferSpace(glm::ivec2 posAc) const;

    /**
     * @brief Plans an upload transfer
     * @pre  hasFreeTransferSpace() == true
     * @note Must be called between beginStep() and endStep()
     */
    void planUpload(
        glm::ivec2                  posCh,
        glm::ivec2                  posAt,
        const std::vector<uint8_t>& tiles,
        std::span<const uint8_t>    branchesSerialized
    );

    /**
     * @brief Plans a download transfer
     * @pre  hasFreeTransferSpace() == true
     * @note Must be called between beginStep() and endStep()
     */
    void planDownload(glm::ivec2 posCh, glm::ivec2 posAt, glm::uint branchReadBuf);

    /**
     * @brief Peforms all previously planned transfers
     */
    void endStep(
        const re::CommandBuffer& cmdBuf,
        const re::Texture&       worldTex,
        const re::Buffer&        branchBuf
    );

    void downloadBranchAllocRegister(
        const re::CommandBuffer& cmdBuf, const re::Buffer& branchBuf
    );

private:
    static constexpr auto k_stageSlotCount   = 16;
    static constexpr auto k_branchStageCount = 512;

    struct StageBuf {
        std::array<std::array<uint8_t, k_chunkByteSize>, k_stageSlotCount> tiles;
        std::array<uint8_t, k_branchStageCount * sizeof(BranchSerialized)> branches;
    };

    struct Stage {
        Stage() { reset(); }

        struct Slot {
            glm::ivec2 targetCh;
            size_t     branchBytes;
        };

        std::array<Slot, k_stageSlotCount> slots;
        /**
         * @brief Uploads are placed at the beginning, downloads at the end
         */
        std::array<vk::BufferImageCopy2, k_stageSlotCount> tileCopyRegions;
        /**
         * @brief Uploads are placed at the beginning, downloads at the end
         */
        std::array<vk::BufferCopy2, k_stageSlotCount * BranchSerialized::memberCount()>
            branchCopyRegions;
        /**
         * @brief Is the stage buffer for tile uploads and downloads
         */
        re::BufferMapped<StageBuf> buf{re::BufferCreateInfo{
            .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                          vma::AllocationCreateFlagBits::eHostAccessRandom,
            .sizeInBytes = sizeof(StageBuf),
            .usage       = vk::BufferUsageFlagBits::eTransferSrc |
                     vk::BufferUsageFlagBits::eTransferDst,
            .debugName = "rw::ChunkTransferMgr::stage::buf"}};

        int  nextUploadSlot;
        int  nextDownloadSlot;
        int  nextBranchUploadSlot;
        int  nextBranchDownloadSlot;
        int  nextBranchUploadByte;
        int  nextBranchDownloadByte;
        void reset();
        bool hasFreeTransferSpace(int branchCount) const;
    };
    re::StepDoubleBuffered<Stage> m_stage;

    int allocIndex(glm::ivec2 posAc) const;
    int branchCount(glm::ivec2 posAc) const;

    re::Pipeline                          m_saveVegetationPl;
    re::BufferMapped<BranchAllocRegister> m_regBuf{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = sizeof(BranchAllocRegister),
        .usage       = vk::BufferUsageFlagBits::eTransferDst,
        .debugName   = "rw::ChunkTransferMgr::reg"}};
};

} // namespace rw
