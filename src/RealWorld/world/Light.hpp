#pragma once
#include <RealEngine/graphics/Vertex.hpp>

struct Light {
	Light(const glm::vec2& posPx, RE::Color col) : posPx(posPx), col(col) {}

	glm::vec2 posPx;	//Position of the center of the light in pixels
	RE::Color col;		//RGB = color of the light, A = strength of the light
};