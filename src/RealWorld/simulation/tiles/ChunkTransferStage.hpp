/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <concepts>

#include <vulkan/vulkan.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/utility/OffsetOfArr.hpp>

#include <RealWorld/simulation/vegetation/BranchRange.hpp>
#include <RealWorld/simulation/vegetation/BranchSerialized.hpp>
#include <RealWorld/simulation/vegetation/shaders/BranchAllocReqUB_glsl.hpp>

namespace rw {

template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
struct StageBuf {
    std::array<std::array<uint8_t, k_chunkByteSize>, k_stageSlotCount> tiles;
    std::array<uint8_t, k_branchStageCount * sizeof(BranchSerialized)> branches;
    glsl::BranchAllocReqUB branchAllocReq{};
};

struct TransferSlot {
    glm::ivec2 targetCh{};
    int branchCount{};
    int branchByteOffset{};
};

/**
 * @brief Is used within ChunkTransferMgr to manage double buffered transfers
 */
template<uint32_t k_stageSlotCount, uint32_t k_branchStageCount>
class ChunkTransferStage {
public:
    using StgBuf = StageBuf<k_stageSlotCount, k_branchStageCount>;

    ChunkTransferStage() {
        m_buf->branchAllocReq.allocSlotsEnd     = 0;
        m_buf->branchAllocReq.deallocSlotsBegin = k_stageSlotCount;
    }

    void setTarget(glm::ivec2 worldTexCh) {
        m_buf->branchAllocReq.worldTexCh = worldTexCh;
    }

    void reset() {
        m_upSlotsEnd                            = 0;
        m_downSlotsBegin                        = k_stageSlotCount;
        m_buf->branchAllocReq.allocSlotsEnd     = 0;
        m_buf->branchAllocReq.deallocSlotsBegin = k_stageSlotCount;
        m_branchUpSlotsEnd                      = 0;
        m_branchDownSlotsBegin                  = m_branchCopyRegions.size();
        m_branchUpBytesEnd                      = 0;
        m_branchDownBytesBegin                  = sizeof(StgBuf::branches);
    }

    bool allocsOrDeallocsPlanned() {
        return m_buf->branchAllocReq.allocSlotsEnd != 0 ||
               m_buf->branchAllocReq.deallocSlotsBegin != k_stageSlotCount;
    }

    int numberOfUploads() const { return m_upSlotsEnd; }
    int numberOfDownloads() const {
        return k_stageSlotCount - m_downSlotsBegin;
    }
    int numberOfBranchUploads() const { return m_branchUpSlotsEnd; }
    int numberOfBranchDownloads() const {
        return m_branchCopyRegions.size() - m_branchDownSlotsBegin;
    }

    void forEachBranchAllocation(std::invocable<glm::ivec2> auto func) const {
        const auto& req = m_buf->branchAllocReq;
        for (int i = 0; i < req.allocSlotsEnd; ++i) { func(req.targetCh[i]); }
    }

    void forEachUpload(std::invocable<int> auto func) const {
        for (int i = 0; i < m_upSlotsEnd; ++i) { func(i); }
    }

    void forEachDownload(std::invocable<int> auto func) const {
        for (int i = m_downSlotsBegin; i < k_stageSlotCount; ++i) { func(i); }
    }

    TransferSlot slot(int i) const { return m_slots[i]; }

    const uint8_t* tiles(int i) const { return m_buf->tiles[i].data(); }

    std::span<const uint8_t> branchesSerializedSpan(TransferSlot slt) const {
        return std::span{
            m_buf->branches.begin() + slt.branchByteOffset,
            m_buf->branches.begin() + slt.branchByteOffset +
                slt.branchCount * sizeof(BranchSerialized)
        };
    }

    [[nodiscard]] bool insertBranchAllocation(glm::ivec2 posCh, int branchCount) {
        assert(branchCount > 0);
        if (!hasFreeRequestSpace()) {
            return false;
        }
        // Request allocation
        auto& reqSlot = m_buf->branchAllocReq.allocSlotsEnd;
        m_buf->branchAllocReq.targetCh[reqSlot]    = posCh;
        m_buf->branchAllocReq.branchCount[reqSlot] = branchCount;
        reqSlot++;
        return true;
    }

    [[nodiscard]] bool insertUpload(
        glm::ivec2 posCh, glm::ivec2 posAt, BranchRange range,
        const uint8_t* tiles, std::span<const uint8_t> branchesSerialized
    ) {
        if (!hasFreeTransferSpace(range.count)) {
            return false;
        }

        auto& upSlotsEnd = m_upSlotsEnd;
        std::memcpy(&m_buf->tiles[upSlotsEnd], tiles, k_chunkByteSize);
        m_tileCopyRegions[upSlotsEnd] = vk::BufferImageCopy2{
            RE_OFFSET_OF_ARR(StgBuf, tiles, upSlotsEnd), // Buffer offset
            0u,
            0u,                                          // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, k_tileLayerCount}, // Subresource
            {posAt.x, posAt.y, 0u},      // Offset
            {iChunkTi.x, iChunkTi.y, 1u} // Extent
        };

        if (range.count > 0) {
            std::memcpy(
                &m_buf->branches[m_branchUpBytesEnd], branchesSerialized.data(),
                branchesSerialized.size_bytes()
            );
            // Plan upload of branches
            vk::DeviceSize sizeBytes = sizeof(BranchSerialized::raster) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, raster, range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::densityStiffness) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, densityStiffness, range.begin),
                sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::lengthTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, lengthTi, range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::radiusTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, radiusTi, range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::angVel) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, angVel, range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, relRestAngNorm, range.begin),
                sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) *
                        range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, parentOffset15wallType31, range.begin),
                sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::absAngNorm) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, absAngNorm[0], range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
            sizeBytes = sizeof(BranchSerialized::absPosTi) * range.count;
            m_branchCopyRegions[m_branchUpSlotsEnd++] = vk::BufferCopy2{
                offsetof(StgBuf, branches) + m_branchUpBytesEnd,
                RE_OFFSET_OF_ARR(glsl::BranchSB, absPosTi[0], range.begin), sizeBytes
            };
            m_branchUpBytesEnd += sizeBytes;
        }

        m_slots[upSlotsEnd] =
            TransferSlot{.targetCh = posCh, .branchCount = range.count};
        upSlotsEnd++;
        return true;
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileUploadRegions() const {
        return {static_cast<uint32_t>(numberOfUploads()), m_tileCopyRegions.data()};
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferCopy2> branchUploadRegions() const {
        return {
            static_cast<uint32_t>(numberOfBranchUploads()),
            m_branchCopyRegions.data()
        };
    }

    [[nodiscard]] bool insertDownloadAndBranchDeallocation(
        glm::ivec2 posCh, glm::ivec2 posAt, BranchRange range
    ) {
        if (!hasFreeTransferSpace(range.count) ||
            (range.count > 0 && !hasFreeRequestSpace())) {
            return false;
        }

        // Plan donwload of tiles
        auto downSlotsBegin               = --m_downSlotsBegin;
        m_tileCopyRegions[downSlotsBegin] = vk::BufferImageCopy2{
            RE_OFFSET_OF_ARR(StgBuf, tiles, downSlotsBegin), // Buffer offset
            0u,
            0u,                                              // Tightly packed
            {vk::ImageAspectFlagBits::eColor, 0u, 0u, k_tileLayerCount}, // Subresource
            {posAt.x, posAt.y, 0u},      // Offset
            {iChunkTi.x, iChunkTi.y, 1u} // Extent
        };

        if (range.count > 0) {
            // Plan download of branches
            vk::DeviceSize sizeBytes = sizeof(BranchSerialized::absPosTi) *
                                       range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, absPosTi[0], range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::absAngNorm) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, absAngNorm[0], range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) *
                        range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, parentOffset15wallType31, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, relRestAngNorm, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::angVel) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, angVel, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::radiusTi) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, radiusTi, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::lengthTi) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, lengthTi, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::densityStiffness) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, densityStiffness, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };
            sizeBytes = sizeof(BranchSerialized::raster) * range.count;
            m_branchDownBytesBegin -= sizeBytes;
            m_branchCopyRegions[--m_branchDownSlotsBegin] = vk::BufferCopy2{
                RE_OFFSET_OF_ARR(glsl::BranchSB, raster, range.begin),
                offsetof(StgBuf, branches) + m_branchDownBytesBegin, sizeBytes
            };

            { // Request deallocation
                auto reqSlot = --m_buf->branchAllocReq.deallocSlotsBegin;
                m_buf->branchAllocReq.targetCh[reqSlot] = posCh;
            }
        }
        m_slots[downSlotsBegin] = TransferSlot{
            .targetCh         = posCh,
            .branchCount      = range.count,
            .branchByteOffset = m_branchDownBytesBegin
        };
        return true;
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferImageCopy2> tileDownloadRegions() const {
        return {
            static_cast<uint32_t>(numberOfDownloads()),
            m_tileCopyRegions.data() + m_downSlotsBegin
        };
    }

    vk::ArrayProxyNoTemporaries<const vk::BufferCopy2> branchDownloadRegions() const {
        return {
            static_cast<uint32_t>(numberOfBranchDownloads()),
            m_branchCopyRegions.data() + m_branchDownSlotsBegin
        };
    }

    const re::Buffer& buffer() const { return m_buf; }

private:
    bool hasFreeTransferSpace(int branchCount) const {
        bool slotsAvailable = m_upSlotsEnd < m_downSlotsBegin;
        bool branchBytesAvailable =
            m_branchUpBytesEnd + (branchCount * sizeof(BranchSerialized)) <=
            m_branchDownBytesBegin;
        return slotsAvailable && branchBytesAvailable;
    }

    bool hasFreeRequestSpace() const {
        return m_buf->branchAllocReq.allocSlotsEnd <
               m_buf->branchAllocReq.deallocSlotsBegin;
    }

    std::array<TransferSlot, k_stageSlotCount> m_slots;

    /**
     * @brief Is the actually stage buffer for tile and branch transfers
     */
    re::BufferMapped<StgBuf> m_buf{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = sizeof(StgBuf),
        .usage       = vk::BufferUsageFlagBits::eTransferSrc |
                 vk::BufferUsageFlagBits::eTransferDst,
        .debugName = "rw::ChunkTransferMgr::stage::buf"
    }};

    /**
     * @brief Uploads are placed at the beginning, downloads at the end
     */
    std::array<vk::BufferImageCopy2, k_stageSlotCount> m_tileCopyRegions;

    /**
     * @brief Uploads are placed at the beginning, downloads at the end
     */
    std::array<vk::BufferCopy2, k_stageSlotCount * BranchSerialized::memberCount()>
        m_branchCopyRegions;

    int m_upSlotsEnd           = 0;
    int m_downSlotsBegin       = k_stageSlotCount;
    int m_branchUpSlotsEnd     = 0;
    int m_branchDownSlotsBegin = m_branchCopyRegions.size();
    int m_branchUpBytesEnd     = 0;
    int m_branchDownBytesBegin = sizeof(StgBuf::branches);
};

} // namespace rw
