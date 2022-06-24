﻿/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/MinimapDrawer.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Viewport.hpp>


MinimapDrawer::MinimapDrawer() {
	//Init VAO
	GLuint vboBindingPoint = 0u;
	m_pouvArr.setBindingPoint(vboBindingPoint, m_pouvBuf, 0u, sizeof(VertexPOUV));
	m_pouvArr.setAttribute(RE::ATTR_POSITION, RE::VertexComponentCount::XY, RE::VertexComponentType::FLOAT, offsetof(VertexPOUV, position));
	m_pouvArr.setAttribute(RE::ATTR_UV, RE::VertexComponentCount::XY, RE::VertexComponentType::FLOAT, offsetof(VertexPOUV, uv));
	m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);

	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_minimapShd, 0u);
}

MinimapDrawer::~MinimapDrawer() {

}

void MinimapDrawer::setTarget(const glm::uvec2& worldDimTi, const glm::vec2& viewSizePx) {
	m_worldDimTi = worldDimTi;
	updateArrayBuffers(viewSizePx);
}

void MinimapDrawer::resizeView(const glm::uvec2& viewSizePx) {
	updateArrayBuffers(viewSizePx);
}

void MinimapDrawer::draw() {
	m_pouvArr.bind();
	m_minimapShd.use();
	m_pouvArr.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4);
	m_minimapShd.unuse();
	m_pouvArr.unbind();
}

void MinimapDrawer::updateArrayBuffers(const glm::vec2& viewSizePx) {
	VertexPOUV vertices[4];

	//Minimap rectangle
	float scale = glm::min(viewSizePx.x / m_worldDimTi.x, viewSizePx.y / m_worldDimTi.y) * 0.5f;
	const glm::vec2 middle = viewSizePx * 0.5f;
	const glm::vec2 world = glm::vec2(m_worldDimTi) * scale;
	int i = 0;
	vertices[i++] = VertexPOUV{{middle.x - world.x, middle.y - world.y}, {0.0f, 0.0f}};
	vertices[i++] = VertexPOUV{{middle.x + world.x, middle.y - world.y}, {1.0f, 0.0f}};
	vertices[i++] = VertexPOUV{{middle.x - world.x, middle.y + world.y}, {0.0f, 1.0f}};
	vertices[i++] = VertexPOUV{{middle.x + world.x, middle.y + world.y}, {1.0f, 1.0f}};

	m_pouvBuf.overwrite(0, sizeof(vertices), vertices);
}