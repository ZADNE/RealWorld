/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/commands/CommandBuffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/constants/world.hpp>
#include <RealWorld/main/ActionCmdBuf.hpp>
#include <RealWorld/vegetation/BranchAllocRegSB.hpp>
#include <RealWorld/world/ChunkTransferStage.hpp>

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
    void setTarget(glm::ivec2 worldTexCh);

    /**
     * @brief Saves all chunks, keeps them in the memory.
     * @note This operation is slow!
     * @return True if successful, false otherwise.
     */
    bool saveChunks(
        const re::Texture& worldTex, const re::Buffer& branchBuf,
        glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr
    );

    /**
     * @brief Finishes transfers from previous step
     * @return Number of finished uploads (= number of transparent changes)
     */
    int beginStep(glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr);

    enum class UploadPlan {
        NoUploadSpace,
        AllocationPlanned,
        UploadPlanned
    };

    /**
     * @brief Plans an upload transfer
     * @note Must be called between beginStep() and endStep()
     * @return Result of the planning
     */
    [[nodiscard]] UploadPlan planUpload(
        glm::ivec2 posCh, glm::ivec2 posAt, const std::vector<uint8_t>& tiles,
        std::span<const uint8_t> branchesSerialized
    );

    enum class DownloadPlan {
        NoDownloadSpace,
        DownloadPlanned
    };

    /**
     * @brief Plans a download transfer
     * @note Must be called between beginStep() and endStep()
     * @return Result of the planning
     */
    [[nodiscard]] DownloadPlan planDownload(glm::ivec2 posCh, glm::ivec2 posAt);

    /**
     * @brief Peforms all previously planned transfers
     */
    void endStep(
        const ActionCmdBuf& acb, const re::Texture& worldTex,
        const re::Buffer& branchBuf, const re::Buffer& branchAllocRegBuf,
        glm::ivec2 worldTexMaskCh, bool externalBranchAllocChanges
    );

    const re::Buffer& allocReqBuf() const { return m_allocReqBuf; }

private:
    int branchCount(glm::ivec2 posAc) const;
    int allocIndex(glm::ivec2 posAc) const;

    static constexpr auto k_stageSlotCount = k_chunkTransferSlots;
    re::StepDoubleBuffered<ChunkTransferStage<k_stageSlotCount, 512>> m_stage;

    glm::ivec2 m_worldTexCh{};

    re::Pipeline m_reallocBranchesPl;
    re::BufferMapped<BranchAllocRegSB> m_regBuf{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = sizeof(BranchAllocRegSB),
        .usage       = vk::BufferUsageFlagBits::eTransferDst,
        .debugName   = "rw::ChunkTransferMgr::reg"
    }};

    re::Buffer m_allocReqBuf{re::BufferCreateInfo{
        .sizeInBytes = sizeof(BranchAllocReqUB),
        .usage       = vk::BufferUsageFlagBits::eTransferDst |
                 vk::BufferUsageFlagBits::eUniformBuffer,
        .debugName = "rw::ChunkTransferMgr::allocReq"
    }};
};

} // namespace rw
