#pragma once
#include <cstdint>

//Physics steps per second
const int PHYSICS_STEPS_PER_SECOND = 50;

//Single block dimensions
#define uvec2_BLOCK_SIZE glm::uvec2(16u, 16u)
#define ivec2_BLOCK_SIZE glm::ivec2(16, 16)
#define vec2_BLOCK_SIZE glm::vec2(16.0f, 16.0f)

const float NUMBER_OF_TILES = 256.0f;
const float NUMBER_OF_TILE_VARIATIONS = 256.0f;

namespace light {
const unsigned int MAX_RANGE = 30u;
}


enum class BLOCK_ID : uint8_t {
	AIR = 0, STONE = 1, DIRT = 2, GRASS = 3,
	BEDROCK = 4, LEAVES = 5, TECHBLOCK = 6, COAL = 7,
	HEMATITE = 8, BAUXITE = 9, COPPER_ORE = 10, CASSITERITE = 11,
	GOLD_ORE = 12, SPODUMENE = 13, QUARTZ_SANDSTONE = 14, BRICK = 15,
	RAW_WOOD = 16, WOODEN_PLANKS = 17, COLD_STONE = 18
};

enum class WALL_ID : uint8_t {
	AIR = 0, STONE = 1, WOOD = 2, DIRT = 3,
	LEAVES = 4, COLD_STONE = 5
};