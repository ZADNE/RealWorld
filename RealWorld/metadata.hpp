#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

//Physics steps per second
const int PHYSICS_STEPS_PER_SECOND = 50;

/**
 * @brief Dimensions of a tile, in pixels
*/
const glm::uvec2 uTILE_SIZE = glm::uvec2(4u, 4u);
const glm::ivec2 iTILE_SIZE = uTILE_SIZE;
const glm::vec2 TILE_SIZE = uTILE_SIZE;


enum class BLOCK : uint8_t {
	AIR, STONE, DIRT, GRASS,
	COLD_STONE, SAND, COLD_DIRT, COLD_GRASS,
	MUD, MUD_GRASS, DRY_GRASS,

	TECHBLOCK = 255
};

enum class WALL : uint8_t {
	AIR, STONE, DIRT, GRASS,
	COLD_STONE, SAND, COLD_DIRT, COLD_GRASS,
	MUD, MUD_GRASS, DRY_GRASS
};