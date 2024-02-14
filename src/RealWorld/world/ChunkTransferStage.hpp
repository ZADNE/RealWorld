/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <concepts>

#include <vulkan/vulkan.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>

#include <RealWorld/vegetation/BranchAllocReqUB.hpp>
#include <RealWorld/vegetation/BranchRange.hpp>
#include <RealWorld/vegetation/BranchSB.hpp>

namespace rw {

template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
struct StageBuf {
    std::array<uint8_t, k_chunkByteSize> tiles[k_stageSlotCount];
    std::array<uint8_t, k_branchStageCount * sizeof(BranchSerialized)> branches;
};

struct StageSlot {
    glm::ivec2 targetCh{};
    int        branchCount{};
    int        branchByteOffset{};
};

/**
 * @brief Is used within ChunkTransferMgr to manage double buffered transfers
 */
template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
class ChunkTransferStage {
public:
    using StageBuf = StageBuf<k_stageSlotCount, k_branchStageCount>;

    ChunkTransferStage() { reset(); }

    void reset() {
        m_upSlotsEnd           = 0;
        m_downSlotsBegin       = k_stageSlotCount;
        m_branchUpSlotsEnd     = 0;
        m_branchDownSlotsBegin = m_branchCopyRegions.size();
        m_branchUpBytesEnd     = 0;
        m_branchDownBytesBegin = sizeof(StageBuf::branches);
    }

    int numberOfUploads() const { return m_upSlotsEnd; }
    int numberOfDownloads() const {
        return k_stageSlotCount - m_downSlotsBegin;
    }
    int numberOfBranchUploads() const { return m_branchUpSlotsEnd; }
    int numberOfBranchDownloads() const {
        return m_branchCopyRegions.size() - m_branchDownSlotsBegin;
    }

    void forEachUpload(std::invocable<int> auto func) const {
        for (int i = 0; i < m_upSlotsEnd; ++i) { func(i); }
    }

    void forEachDownload(std::invocable<int> auto func) const {
        for (int i = m_downSlotsBegin; i < k_stageSlotCount; ++i) { func(i); }
    }

    StageSlot slot(int i) const { return m_slots[i]; }

    const uint8_t* tiles(int i) const { return m_buf->tiles[i].data(); }

    std::span<const uint8_t> branchesSerializedSpan(StageSlot slt) const {
        return std::span{
            m_buf->branches.begin() + slt.branchByteOffset,
            m_buf->branches.begin() + slt.branchByteOffset +
                slt.branchCount * sizeof(BranchSerialized)};
    }

    bool hasFreeTransferSpace(int branchCount) const {
        bool slotsAvailable = m_upSlotsEnd < m_downSlotsBegin;
        bool branchBytesAvailable =
            m_branchUpBytesEnd + (branchCount * sizeof(BranchSerialized)) <=
            m_branchDownBytesBegin;
        return slotsAvailable && branchBytesAvailable;
    }

    void insertUpload(
        glm::ivec2               posCh,
        glm::ivec2               posAt,
        BranchRange              range,
        glm::uint                branchReadBuf,
        const uint8_t*           tiles,
        std::span<const uint8_t> branchesSerialized
    ) {
        auto& upSlotsEnd = m_upSlotsEnd;
        std::memcpy(&m_buf->tiles[upSlotsEnd], tiles, k_chunkByteSize);
        m_tileCopyRegions[upSlotsEnd] = vk::BufferImageCopy2{
            offsetof(StageBuf, tiles[upSlotsEnd]), // Buffer offset
            0u,
            0u,                                            // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u}, // Subresource
            {posAt.x, posAt.y, 0u},                        // Offset
            {iChunkTi.x, iChunkTi.y, 1u}                   // Extent
        };

        if (range.count > 0) {
            std::memcpy(
                &m_buf->branches[m_branchUpBytesEnd],
                branchesSerialized.data(),
                branchesSerialized.size_bytes()
            );
            // Plan upload of branches
            vk::DeviceSize sizeBytes = sizeof(BranchSerialized::raster) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, raster[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::densityStiffness) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, densityStiffness[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::lengthTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, lengthTi[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::radiusTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, radiusTi[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::angVel) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, angVel[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, relRestAngNorm[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) *
                        range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, parentOffset15wallType31[range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::absAngNorm) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, absAngNorm[branchReadBuf][range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::absPosTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StageBuf, branches) + m_branchUpBytesEnd,
                offsetof(BranchSB, absPosTi[branchReadBuf][range.begin]),
                sizeBytes};
            m_branchUpBytesEnd += sizeBytes;
        }

        m_slots[upSlotsEnd] = StageSlot{.targetCh = posCh, .branchCount = range.count};
        upSlotsEnd++;
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileUploadRegions() const {
        return {static_cast<uint32_t>(numberOfUploads()), m_tileCopyRegions.data()};
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferCopy2> branchUploadRegions() const {
        return {
            static_cast<uint32_t>(numberOfBranchUploads()),
            m_branchCopyRegions.data()};
    }

    void insertDownload(
        glm::ivec2 posCh, glm::ivec2 posAt, BranchRange range, glm::uint branchReadBuf
    ) {
        auto downSlotsBegin               = --m_downSlotsBegin;
        m_tileCopyRegions[downSlotsBegin] = vk::BufferImageCopy2{
            offsetof(StageBuf, tiles[downSlotsBegin]), // Buffer offset
            0u,
            0u,                                            // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u}, // Subresource
            {posAt.x, posAt.y, 0u},                        // Offset
            {iChunkTi.x, iChunkTi.y, 1u}                   // Extent
        };

        if (range.count > 0) {
            // Plan download of branches
            vk::DeviceSize sizeBytes = sizeof(BranchSerialized::absPosTi) *
                                       range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, absPosTi[branchReadBuf][range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::absAngNorm) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, absAngNorm[branchReadBuf][range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) *
                        range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, parentOffset15wallType31[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, relRestAngNorm[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::angVel) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, angVel[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::radiusTi) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, radiusTi[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::lengthTi) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, lengthTi[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::densityStiffness) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, densityStiffness[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
            sizeBytes = sizeof(BranchSerialized::raster) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                offsetof(BranchSB, raster[range.begin]),
                offsetof(StageBuf, branches) + m_branchDownBytesBegin,
                sizeBytes};
        }
        m_slots[downSlotsBegin] = StageSlot{
            .targetCh         = posCh,
            .branchCount      = range.count,
            .branchByteOffset = m_branchDownBytesBegin};
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileDownloadRegions() const {
        return {
            static_cast<uint32_t>(numberOfDownloads()),
            m_tileCopyRegions.data() + m_downSlotsBegin};
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferCopy2> branchDownloadRegions() const {
        return {
            static_cast<uint32_t>(numberOfBranchDownloads()),
            m_branchCopyRegions.data() + m_branchDownSlotsBegin};
    }

    [[nodiscard]] BranchAllocReqUB composeBranchAllocRequest(glm::ivec2 worldTexCh
    ) const {
        BranchAllocReqUB req;
        auto             setReq = [&](int i) {
            auto slt           = slot(i);
            req.targetCh[i]    = slt.targetCh;
            req.branchCount[i] = slt.branchCount;
        };
        forEachUpload(setReq);
        forEachDownload(setReq);
        req.worldTexCh         = worldTexCh;
        req.uploadSlotsEnd     = m_upSlotsEnd;
        req.downloadSlotsBegin = m_downSlotsBegin;
        return req;
    }

    const re::Buffer& buffer() const { return m_buf; }

private:
    int m_upSlotsEnd;
    int m_downSlotsBegin;
    int m_branchUpSlotsEnd;
    int m_branchDownSlotsBegin;
    int m_branchUpBytesEnd;
    int m_branchDownBytesBegin;

    std::array<StageSlot, k_stageSlotCount> m_slots;

    /**
     * @brief Is the actually stage buffer for tile and branch transfers
     */
    re::BufferMapped<StageBuf> m_buf{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = sizeof(StageBuf),
        .usage       = vk::BufferUsageFlagBits::eTransferSrc |
                 vk::BufferUsageFlagBits::eTransferDst,
        .debugName = "rw::ChunkTransferMgr::stage::buf"}};

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