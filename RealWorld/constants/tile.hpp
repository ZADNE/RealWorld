#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

//Physics steps per second
const int PHYSICS_STEPS_PER_SECOND = 100;

/**
 * @brief Dimensions of a tile, in pixels
*/
const glm::uvec2 uTILEPx = glm::uvec2(4u, 4u);
const glm::ivec2 iTILEPx = uTILEPx;
const glm::vec2 TILEPx = uTILEPx;


enum class BLOCK : uint8_t {
	AIR, WATER, LAVA, ACID,
	STONE, DIRT, GRASS, COLD_STONE,
	SAND, COLD_DIRT, COLD_GRASS, MUD,
	MUD_GRASS, DRY_GRASS,

	LAST_FLUID = ACID,
	TECHBLOCK = 255
};
const unsigned int LAST_FLUID = static_cast<unsigned int>(BLOCK::LAST_FLUID);

enum class WALL : uint8_t {
	AIR, STONE, DIRT, GRASS,
	COLD_STONE, SAND, COLD_DIRT, COLD_GRASS,
	MUD, MUD_GRASS, DRY_GRASS
};

enum class TILE_VALUE : unsigned int {
	BLOCK = 0, BLOCK_VAR = 1, WALL = 2, WALL_VAR = 3
};

enum class SET_TARGET : unsigned int {
	BLOCK = static_cast<unsigned int>(TILE_VALUE::BLOCK),
	WALL = static_cast<unsigned int>(TILE_VALUE::WALL)
};