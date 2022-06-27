/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/Vertex.hpp>

 /**
  * @brief Represents a dynamic light that can be added into the world.
 */
struct Light {
	Light(const glm::vec2& posPx, RE::Color col) : posPx(posPx), col(col) {}

	glm::vec2 posPx;	//Position of the center of the light in pixels
	RE::Color col;		//RGB = color of the light, A = strength of the light
};
