#include <RealWorld/world/WorldCreator.hpp>

#include <RealWorld/world/chunk/Chunk.hpp>

WorldData WorldCreator::createWorld(std::string worldName, int seed) {
	WorldData wd;
	//World info
	wd.wi.seed = seed;
	wd.wi.worldName = worldName;

	//Player data
	wd.pd.pos = CHUNK_SIZE * glm::ivec2(0, 5) * iTILEPx;
	wd.pd.id.resize({10, 4});
	wd.pd.id.items[0][0] = Item{I_ID::STEEL_PICKAXE, 1};
	wd.pd.id.items[1][0] = Item{I_ID::STEEL_HAMMER, 1};

	wd.pd.id.items[2][0] = Item{I_ID::B_STONE, 1};
	wd.pd.id.items[3][0] = Item{I_ID::B_LAVA, 1};
	wd.pd.id.items[4][0] = Item{I_ID::B_GRASS, 1};
	wd.pd.id.items[5][0] = Item{I_ID::B_WATER, 1};

	return wd;
}
