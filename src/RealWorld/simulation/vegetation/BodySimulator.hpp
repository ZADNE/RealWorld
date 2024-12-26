/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/simulation/vegetation/shaders/BodiesSB_glsl.hpp>

namespace rw {

/**
 * @brief Is not used, maybe in fufure...
 */
class BodySimulator {
public:
    explicit BodySimulator(const re::PipelineLayout& simulationPL);

    void step(const re::CommandBuffer& cb);

    const re::Buffer& adoptSave(glm::ivec2 worldTexSizeCh);

private:

    /**
     * @brief Size of header is same as 1 body
     */
    static constexpr int k_bodyHeaderSize = sizeof(glsl::BodiesSB) /
                                            sizeof(glsl::Body);
    static_assert(k_bodyHeaderSize * sizeof(glsl::Body) == sizeof(glsl::BodiesSB));

    re::Buffer m_bodiesBuf;
    re::Pipeline m_simulateBodiesPl;
};

} // namespace rw
