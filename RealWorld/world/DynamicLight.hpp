#pragma once

struct DynamicLight {
	DynamicLight(const glm::vec2& posPx, RE::Colour col, GLfloat dir, GLfloat cone) : posPx(posPx), col(col), dir(dir), cone(cone) {}

	glm::vec2 posPx;		//Position of the center of the light in pixels
	RE::Colour col;	//RGB = colour of the light, A = strength of the light
	GLfloat dir;			//[0.0f; 1.0f] range
	GLfloat cone;			//[0.0f; 1.0f] range
};