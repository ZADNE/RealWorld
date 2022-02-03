#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

//Physics steps per second
const int PHYSICS_STEPS_PER_SECOND = 50;

/**
 * @brief Dimensions of a tile, in pixels; must be integer
*/
const glm::vec2 TILE_SIZE = glm::vec2(16.0f, 16.0f);
const glm::ivec2 iTILE_SIZE = TILE_SIZE;
const glm::uvec2 uTILE_SIZE = TILE_SIZE;

namespace light {
const unsigned int MAX_RANGE = 30u;
}


enum class BLOCK : uint8_t {
	AIR,			STONE,			DIRT,			GRASS,
	COLD_STONE,		SAND,			COLD_DIRT,		COLD_GRASS,

	TECHBLOCK = 255
};

enum class WALL : uint8_t {
	AIR,			STONE,			DIRT,			COLD_STONE,
	SAND,			COLD_DIRT
};