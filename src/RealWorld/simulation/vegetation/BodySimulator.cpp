/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/body.hpp>
#include <RealWorld/simulation/vegetation/BodySimulator.hpp>
#include <RealWorld/simulation/vegetation/shaders/simulateBodies_comp.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

BodySimulator::BodySimulator(const re::PipelineLayout& simulationPL)
    : m_simulateBodiesPl{
          {.pipelineLayout = *simulationPL}, {.comp = glsl::simulateBodies_comp}
      } {
}

void BodySimulator::step(const re::CommandBuffer& cb) {
    /*cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateBodiesPl);
    cb->dispatchIndirect(*m_bodiesBuf, offsetof(BodiesSBHeader, dispatchX));*/
}

const re::Buffer& BodySimulator::adoptSave(glm::ivec2 worldTexSizeCh) {
    auto maxBodyCount = k_bodiesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                        k_bodyHeaderSize;

    glsl::BodiesSBHeader initHeader{
        .bodiesDispatchX  = 0,
        .bodiesDispatchY  = 1,
        .bodiesDispatchZ  = 1,
        .currentBodyCount = 0,
        .maxBodyCount     = maxBodyCount
    };

    m_bodiesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(glsl::BodiesSB) + sizeof(glsl::Body) * maxBodyCount,
        .usage    = eStorageBuffer | eIndirectBuffer,
        .initData = re::objectToByteSpan(initHeader)
    }};

    return m_bodiesBuf;
}

} // namespace rw
