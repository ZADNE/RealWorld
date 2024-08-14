/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/simulation/general/ActionCmdBuf.hpp>
#include <RealWorld/simulation/objects/DroppedTilesSB.hpp>

namespace rw {

/**
 * @brief   Simulates movement of dropped tiles.
 * @details Dropped tiles are blocked by placed tiles. They move towards the
 *          player if he is close enough, otherwise they fall to the ground.
 */
class DroppedTilesMgr {
public:
    explicit DroppedTilesMgr(const re::PipelineLayout& pipelineLayout);

    void step(const ActionCmdBuf& acb);

    const re::Buffer& droppedTilesBuf() { return m_tilesBuf; }

private:
    constexpr static DroppedTilesSBHeader k_initHeader{};
    re::Buffer m_tilesBuf{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(DroppedTilesSB),
        .usage       = vk::BufferUsageFlagBits::eIndirectBuffer |
                 vk::BufferUsageFlagBits::eStorageBuffer |
                 vk::BufferUsageFlagBits::eVertexBuffer,
        .initData  = re::objectToByteSpan(k_initHeader),
        .debugName = "rw::DroppedTilesMgr::tilesBuf"
    }};
    re::Pipeline m_moveDroppedTilesPl;
};

} // namespace rw
