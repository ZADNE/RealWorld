#pragma once
#include <glm/vec2.hpp>

struct WorldInfo {
	glm::uvec2 chunkDims;
	int seed;
	std::string worldName;
};