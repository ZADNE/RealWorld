/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/bodies.hpp>
#include <RealWorld/world/BodySimulator.hpp>
#include <RealWorld/world/shaders/simulateBodies_comp.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

BodySimulator::BodySimulator(const re::PipelineLayout& simulationPL)
    : m_simulateBodiesPl{
          {.pipelineLayout = *simulationPL}, {.comp = simulateBodies_comp}} {
}

void BodySimulator::step(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateBodiesPl);
    commandBuffer.dispatchIndirect(**m_bodiesBuf, offsetof(BodiesSBHeader, dispatchX));
}

const re::Buffer& BodySimulator::adoptSave(
    const MetadataSave& save, const glm::ivec2& worldTexSizeCh
) {
    const glm::ivec2 maxChunksInRow = worldTexSizeCh - 1;
    auto maxBodyCount = k_bodiesPerChunk * maxChunksInRow.x * maxChunksInRow.y;

    BodiesSBHeader initHeader{
        .dispatchY        = k_simulateBodiesGroupSize.y,
        .dispatchZ        = k_simulateBodiesGroupSize.z,
        .currentBodyCount = 0,
        .maxBodyCount     = maxBodyCount};

    m_bodiesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Body) * maxBodyCount,
        .usage       = eStorageBuffer | eIndirectBuffer,
        .initData    = re::objectToByteSpan(initHeader)}};

    return *m_bodiesBuf;
}

} // namespace rw
