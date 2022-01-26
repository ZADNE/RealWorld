#pragma once

#include <RealEngine/graphics/Vertex.hpp>

struct VertexPOUV {
	VertexPOUV() {}

	VertexPOUV(const glm::vec2& position, const glm::vec2& uv) :
		position(position), uv(uv) {}

	VertexPOUV(float x, float y, float u, float v) :
		position(x, y), uv(u, v) {}

	void setPosition(float x, float y) {
		position.x = x;
		position.y = y;
	}

	void setPosition(const glm::vec2& pos) {
		position = pos;
	}

	void setUV(float u, float v) {
		uv.x = u;
		uv.y = v;
	}

	glm::vec2 position;
	glm::vec2 uv;
};