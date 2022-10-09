﻿/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/MinimapDrawer.hpp>

#include <RealEngine/rendering/vertices/Vertex.hpp>
#include <RealEngine/rendering/output/Viewport.hpp>

#include <RealWorld/reserved_units/buffers.hpp>


template<RE::Renderer R>
MinimapDrawer<R>::MinimapDrawer() {
    //Init VAO
    unsigned int vboBindingPoint = 0u;
    m_pouvArr.setBindingPoint(vboBindingPoint, m_pouvBuf, 0u, sizeof(VertexPOUV));
    m_pouvArr.setAttribute(RE::ATTR_POSITION, RE::VertexComponentCount::XY, RE::VertexComponentType::FLOAT, offsetof(VertexPOUV, position));
    m_pouvArr.setAttribute(RE::ATTR_UV, RE::VertexComponentCount::XY, RE::VertexComponentType::FLOAT, offsetof(VertexPOUV, uv));
    m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
    m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);

    m_minimapShd.backInterfaceBlock(0u, UNIF_BUF_VIEWPORT_MATRIX);
}

template<RE::Renderer R>
void MinimapDrawer<R>::setTarget(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx) {
    updateArrayBuffers(worldTexSize, viewSizePx);
}

template<RE::Renderer R>
void MinimapDrawer<R>::resizeView(const glm::ivec2& worldTexSize, const glm::uvec2& viewSizePx) {
    updateArrayBuffers(worldTexSize, viewSizePx);
}

template<RE::Renderer R>
void MinimapDrawer<R>::draw() {
    m_pouvArr.bind();
    m_minimapShd.use();
    m_pouvArr.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
    m_minimapShd.unuse();
    m_pouvArr.unbind();
}

template<RE::Renderer R>
void MinimapDrawer<R>::updateArrayBuffers(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx) {
    VertexPOUV vertices[4];

    //Minimap rectangle
    float scale = glm::min(viewSizePx.x / worldTexSize.x, viewSizePx.y / worldTexSize.y) * 0.5f;
    const glm::vec2 middle = viewSizePx * 0.5f;
    const glm::vec2 world = glm::vec2(worldTexSize) * scale;
    int i = 0;
    vertices[i++] = VertexPOUV{{middle.x - world.x, middle.y - world.y}, {0.0f, 0.0f}};
    vertices[i++] = VertexPOUV{{middle.x + world.x, middle.y - world.y}, {1.0f, 0.0f}};
    vertices[i++] = VertexPOUV{{middle.x - world.x, middle.y + world.y}, {0.0f, 1.0f}};
    vertices[i++] = VertexPOUV{{middle.x + world.x, middle.y + world.y}, {1.0f, 1.0f}};

    m_pouvBuf.overwrite(0, sizeof(vertices), vertices);
}

template class MinimapDrawer<RE::RendererVK13>;
template class MinimapDrawer<RE::RendererGL46>;
