/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>

#include <RealWorld/vegetation/BranchSB.hpp>

namespace rw {

/**
 * @brief Manages uploads and downloads of vegetation to/from the world texture
 */
class VegManager {
public:
    explicit VegManager(const re::PipelineLayout& pipelineLayout);

    /**
     * @brief Resets state to no pending transfers
     */
    void reset();

    /**
     * @brief Uploads and downloads can only be planned when there is enough space
     */
    [[nodiscard]] bool hasFreeTransferSpace(glm::ivec2 posAc) const;

    void downloadBranchAllocRegister(
        const re::CommandBuffer& cmdBuf, const re::Buffer& branchBuf
    );

private:
    re::Pipeline m_saveVegetationPl;

    re::BufferMapped<BranchAllocRegister> m_regBuf{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = sizeof(BranchAllocRegister),
        .usage       = vk::BufferUsageFlagBits::eTransferDst,
        .debugName   = "rw::VegManager::bs::reg"}};

    static constexpr auto k_branchStageSlots = 512;
    struct BranchStage {
        int  nextUploadSlot   = 0;
        int  nextDownloadSlot = k_branchStageSlots - 1;
        void reset();
        bool hasFreeTransferSpace(int branchCount) const;
    };
    re::StepDoubleBuffered<BranchStage> m_bs;
};

} // namespace rw
