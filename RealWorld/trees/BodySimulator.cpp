/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/body.hpp>
#include <RealWorld/trees/BodySimulator.hpp>
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

const re::Buffer& BodySimulator::adoptSave(const glm::ivec2& worldTexSizeCh) {
    auto maxBodyCount = k_bodiesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                        k_bodyHeaderSize;

    BodiesSBHeader initHeader{
        .dispatchX        = 0,
        .dispatchY        = 1,
        .dispatchZ        = 1,
        .currentBodyCount = 0,
        .maxBodyCount     = maxBodyCount};

    m_bodiesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(BodiesSBHeader) + sizeof(Body) * maxBodyCount,
        .usage       = eStorageBuffer | eIndirectBuffer,
        .initData    = re::objectToByteSpan(initHeader)}};

    return *m_bodiesBuf;
}

} // namespace rw
