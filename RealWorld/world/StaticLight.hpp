#pragma once
#include <RealEngine/graphics/Vertex.hpp>

struct StaticLight {
	StaticLight(const glm::vec2& posBc, RE::Colour col, float dir, float cone) : posBc(posBc), col(col), dir(dir), cone(cone) {}

	glm::vec2 posBc;	//[glm::vec2(0.0f, 0.0f); worldDimBc] range | MUST BE INTEGER VALUE
	RE::Colour col;		//RGB = colour of the light, A = strength of the light
	float dir;			//[0.0f; 1.0f] range
	float cone;			//[0.0f; 1.0f] range
};