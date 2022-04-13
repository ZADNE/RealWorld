#pragma once
#include <RealEngine/graphics/Vertex.hpp>

struct DynamicLight {
	DynamicLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone) : posPx(posPx), col(col), dir(dir), cone(cone) {}

	glm::vec2 posPx;	//Position of the center of the light in pixels
	RE::Colour col;		//RGB = colour of the light, A = strength of the light
	float dir;		//[0.0f; 1.0f] range
	float cone;		//[0.0f; 1.0f] range
};