/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/vertices/VertexArray.hpp>
#include <RealEngine/rendering/vertices/ShaderProgram.hpp>

#include <RealWorld/shaders/drawing.hpp>

/**
 * @brief Renders minimap of the world
*/
class MinimapDrawer {
public:
    MinimapDrawer();

    void setTarget(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx);
    void resizeView(const glm::ivec2& worldTexSize, const glm::uvec2& viewSizePx);

    void draw();
private:
    void updateArrayBuffers(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx);

    RE::ShaderProgram m_minimapShd{ {.vert = minimap_vert, .frag = minimap_frag} };

    RE::VertexArray m_pouvArr;

    struct VertexPOUV {
        VertexPOUV() {}

        VertexPOUV(const glm::vec2& position, const glm::vec2& uv) :
            position(position), uv(uv) {}

        glm::vec2 position;
        glm::vec2 uv;
    };
    RE::Buffer m_pouvBuf{ sizeof(VertexPOUV) * 4, RE::BufferUsageFlags::DYNAMIC_STORAGE };
};