/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class BodySimulator {
public:
    explicit BodySimulator(const re::PipelineLayout& simulationPL);

    void step(const vk::CommandBuffer& cmdBuf);

    const re::Buffer& adoptSave(glm::ivec2 worldTexSizeCh);

private:
    struct Body {
        glm::ivec2 bottomCenterPx;
        glm::ivec2 sizePx;
        glm::ivec2 velocityPx;
        glm::vec2  rotationRad; /**< y component is unused */
    };

    struct BodiesSBHeader {
        glm::uint dispatchX;
        glm::uint dispatchY;
        glm::uint dispatchZ;
        int       currentBodyCount;
        int       maxBodyCount;
        int       padding[3];
    };

#pragma warning(push)
#pragma warning(disable : 4200)
    struct BodiesSB {
        BodiesSBHeader header;
        Body           bodies[];
    };
#pragma warning(pop)

    /**
     * @brief Size of header is same as 1 body
     */
    static constexpr int k_bodyHeaderSize = sizeof(BodiesSBHeader) / sizeof(Body);
    static_assert(k_bodyHeaderSize * sizeof(Body) == sizeof(BodiesSBHeader));

    re::Buffer   m_bodiesBuf;
    re::Pipeline m_simulateBodiesPl;
};

} // namespace rw