/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class BodySimulator {
public:
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

    explicit BodySimulator(const re::PipelineLayout& simulationPL);

    void step(const vk::CommandBuffer& commandBuffer);

    const re::Buffer& adoptSave(
        const MetadataSave& save, const glm::ivec2& worldTexSizeCh
    );

private:
    /**
     * @brief Size of header is same as 1 body
     */
    static_assert(sizeof(BodiesSBHeader) == sizeof(Body));

    std::optional<re::Buffer> m_bodiesBuf;
    re::Pipeline              m_simulateBodiesPl;
};

} // namespace rw