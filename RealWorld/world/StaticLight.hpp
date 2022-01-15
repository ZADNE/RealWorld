#pragma once

struct StaticLight {
	StaticLight(const glm::vec2& posBc, RE::Colour col, GLfloat dir, GLfloat cone) : posBc(posBc), col(col), dir(dir), cone(cone) {}

	glm::vec2 posBc;		//[glm::vec2(0.0f, 0.0f); worldDimBc] range | MUST BE INTEGER VALUE
	RE::Colour col;			//RGB = colour of the light, A = strength of the light
	GLfloat dir;			//[0.0f; 1.0f] range
	GLfloat cone;			//[0.0f; 1.0f] range
};