#pragma once

#include <RealEngine/graphics/Vertex.hpp>

struct VertexPOUV {
	VertexPOUV() {}

	VertexPOUV(const glm::vec2& position, const glm::vec2& uv) :
		position(position), uv(uv) {}

	glm::vec2 position;
	glm::vec2 uv;
};