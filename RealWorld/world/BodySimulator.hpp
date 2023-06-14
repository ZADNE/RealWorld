/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class BodySimulator {
public:
    struct Body {
        glm::vec2 bottomCenterPx;
        glm::vec2 sizePx;
        glm::vec2 velocityPx;
        glm::vec2 rotationRad; /**< y component is unused */
    };

    struct BodiesSBHeader {
        int currentBodyCount;
        int maxBodyCount;
        int padding[6];
    };

#pragma warning(push)
#pragma warning(disable : 4200)
    struct BodiesSB {
        BodiesSBHeader header;
        Body           bodies[];
    };
#pragma warning(pop)

    const re::Buffer& adoptSave(
        const MetadataSave& save, const glm::ivec2& worldTexSizeCh
    );

private:
    /**
     * @brief Size of header is same as 1 body
     */
    static_assert(sizeof(BodiesSBHeader) == sizeof(Body));

    std::optional<re::Buffer> m_bodiesBuf;
};

} // namespace rw