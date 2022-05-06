/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/common.hpp>

//Physics steps per second
constexpr int PHYSICS_STEPS_PER_SECOND = 100;

/**
 * @brief Dimensions of a tile, in pixels
*/
constexpr glm::uvec2 uTILEPx = glm::uvec2(4u, 4u);
constexpr glm::ivec2 iTILEPx = uTILEPx;
constexpr glm::vec2 TILEPx = uTILEPx;


enum class BLOCK : uint8_t {
	STONE, DIRT, GRASS, COLD_STONE,
	SAND, COLD_DIRT, COLD_GRASS, MUD,
	MUD_GRASS, DRY_GRASS, HALLOW_STONE, HALLOW_DIRT,
	HALLOW_GRASS,

	WATER = 224, LAVA, STEAM,
	FIRE, SMOKE,
	
	ACID = 254,

	AIR = 255
};

enum class WALL : uint8_t {
	STONE, DIRT, GRASS,
	COLD_STONE, SAND, COLD_DIRT, COLD_GRASS,
	MUD, MUD_GRASS, DRY_GRASS,

	AIR = 255
};

enum class TILE_VALUE : unsigned int {
	BLOCK = 0, BLOCK_VAR = 1, WALL = 2, WALL_VAR = 3
};

enum class LAYER : unsigned int {
	BLOCK = static_cast<unsigned int>(TILE_VALUE::BLOCK),
	WALL = static_cast<unsigned int>(TILE_VALUE::WALL)
};

/**
 * @brief Converts a position in pixels to position in tiles
 */
inline glm::vec2 pxToTi(const glm::vec2& posPx) {
	return glm::floor(posPx / TILEPx);
}

/**
 * @brief Converts a position in tiles to position in pixels
 */
inline glm::vec2 tiToPx(const glm::vec2& posTi) {
	return posTi * TILEPx;
}
