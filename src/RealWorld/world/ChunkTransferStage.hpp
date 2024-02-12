/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vulkan/vulkan.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>

#include <RealWorld/vegetation/BranchAllocRequestUB.hpp>
#include <RealWorld/vegetation/BranchSB.hpp>

namespace rw {

template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
struct StageBuf {
    std::array<std::array<uint8_t, k_chunkByteSize>, k_stageSlotCount> tiles;
    std::array<uint8_t, k_branchStageCount * sizeof(BranchSerialized)> branches;
};

struct StageSlot {
    glm::ivec2 targetCh{};
    int        branchCount{};
};

/**
 * @brief Is used within ChunkTransferMgr to manage double buffered transfers
 */
template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
class ChunkTransferStage {
public:
    using StageBuf = StageBuf<k_stageSlotCount, k_branchStageCount>;

    ChunkTransferStage() { reset(); }

    void setTarget(glm::ivec2 worldTexCh) {
        m_allocReq.worldTexCh = worldTexCh;
        reset();
    }

    void reset() {
        m_allocReq.uploadSlotsEnd     = 0;
        m_allocReq.downloadSlotsBegin = k_stageSlotCount;
        nextBranchUploadSlot          = 0;
        nextBranchDownloadSlot        = m_branchCopyRegions.size();
        nextBranchUploadByte          = 0;
        nextBranchDownloadByte        = sizeof(StageBuf::branches);
    }

    int uploadSlotsEnd() const { return m_allocReq.uploadSlotsEnd; }
    int downloadSlotsBegin() const { return m_allocReq.downloadSlotsBegin; }

    StageSlot slot(int i) const {
        return StageSlot{
            .targetCh    = m_allocReq.targetCh[i],
            .branchCount = m_allocReq.branchCount[i]};
    }

    bool hasFreeTransferSpace(int branchCount) const {
        bool slotsAvailable = uploadSlotsEnd() < downloadSlotsBegin();
        bool branchBytesAvailable =
            nextBranchUploadByte + (branchCount * sizeof(BranchSerialized)) <=
            nextBranchDownloadByte;
        return slotsAvailable && branchBytesAvailable;
    }

    void insertUpload(glm::ivec2 posCh, glm::ivec2 posAt, const uint8_t* tiles) {
        auto& upSlotsEnd = m_allocReq.uploadSlotsEnd;
        auto bufOffset = static_cast<vk::DeviceSize>(upSlotsEnd) * k_chunkByteSize;
        std::memcpy(&buf[bufOffset], tiles, k_chunkByteSize);
        m_tileCopyRegions[upSlotsEnd] = vk::BufferImageCopy2{
            bufOffset, // Buffer offset
            0u,
            0u,                                            // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u}, // Subresource
            {posAt.x, posAt.y, 0u},                        // Offset
            {iChunkTi.x, iChunkTi.y, 1u}                   // Extent
        };
        m_allocReq.targetCh[upSlotsEnd]    = posCh;
        m_allocReq.branchCount[upSlotsEnd] = 0;
        upSlotsEnd++;
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileUploadRegions() const {
        return {static_cast<uint32_t>(uploadSlotsEnd()), m_tileCopyRegions.data()};
    }

    void insertDownload(
        glm::ivec2 posCh,
        glm::ivec2 posAt,
        glm::uint  firstBranch,
        glm::uint  branchCount,
        glm::uint  branchReadBuf
    ) {
        auto downSlotsBegin = --m_allocReq.downloadSlotsBegin;
        auto bufOffset      = static_cast<vk::DeviceSize>(downSlotsBegin) *
                         k_chunkByteSize;
        m_tileCopyRegions[downSlotsBegin] = vk::BufferImageCopy2{
            bufOffset, // Buffer offset
            0u,
            0u,                                            // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u}, // Subresource
            {posAt.x, posAt.y, 0u},                        // Offset
            {iChunkTi.x, iChunkTi.y, 1u}                   // Extent
        };

        if (branchCount > 0) {
            // Plan download of branches
            vk::DeviceSize sizeBytes = sizeof(BranchSerialized::absPosTi) *
                                       branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, absPosTi[branchReadBuf][firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::absAngNorm) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, absAngNorm[branchReadBuf][firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) *
                        branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, parentOffset15wallType31[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, relRestAngNorm[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::angVel) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, angVel[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::radiusTi) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, radiusTi[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::lengthTi) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, lengthTi[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::densityStiffness) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, densityStiffness[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::raster) * branchCount;
            nextBranchDownloadByte -= sizeBytes;
            m_branchCopyRegions[--nextBranchDownloadSlot] = vk::BufferCopy2{
                offsetof(BranchSB, raster[firstBranch]),
                offsetof(StageBuf, branches) + nextBranchDownloadByte,
                sizeBytes};
        }
        m_allocReq.targetCh[downSlotsBegin]    = posCh;
        m_allocReq.branchCount[downSlotsBegin] = branchCount;
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileDownloadRegions() const {
        return {
            static_cast<uint32_t>(k_stageSlotCount - downloadSlotsBegin()),
            &m_tileCopyRegions[downloadSlotsBegin()]};
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferCopy2> branchDownloadRegions() const {
        return {
            static_cast<uint32_t>(m_branchCopyRegions.size() - nextBranchDownloadSlot),
            &m_branchCopyRegions[nextBranchDownloadSlot]};
    }

    const BranchAllocRequestUB& branchAllocRequest() const {
        return m_allocReq;
    }
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

    int nextBranchUploadSlot;
    int nextBranchDownloadSlot;
    int nextBranchUploadByte;
    int nextBranchDownloadByte;

private:
    /**
     * @brief Specifies requests for allocation and deallocation
     */
    BranchAllocRequestUB m_allocReq;

    /**
     * @brief Uploads are placed at the beginning, downloads at the end
     */
    std::array<vk::BufferImageCopy2, k_stageSlotCount> m_tileCopyRegions;

    /**
     * @brief Uploads are placed at the beginning, downloads at the end
     */
    std::array<vk::BufferCopy2, k_stageSlotCount * BranchSerialized::memberCount()>
        m_branchCopyRegions;
};

} // namespace rw
