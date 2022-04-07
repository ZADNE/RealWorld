#include <RealWorld/world/WorldCreator.hpp>

#include <RealWorld/world/chunk/Chunk.hpp>

WorldData WorldCreator::createWorld(std::string worldName, int seed) {
	WorldData wd;
	//World info
	wd.wi.seed = seed;
	wd.wi.worldName = worldName;

	//Player data
	wd.pd.pos = iCHUNK_SIZE * glm::ivec2(0, 5) * iTILEPx;
	wd.pd.id.resize({ 10, 4 });
	int x = 0;
	wd.pd.id[x++][0] = Item{ ITEM::STEEL_PICKAXE, 1 };
	wd.pd.id[x++][0] = Item{ ITEM::STEEL_HAMMER, 1 };

	wd.pd.id[x++][0] = Item{ ITEM::B_STONE, 1 };
	wd.pd.id[x++][0] = Item{ ITEM::B_WATER, 1};
	wd.pd.id[x++][0] = Item{ ITEM::B_LAVA, 1 };
	wd.pd.id[x++][0] = Item{ ITEM::B_GRASS, 1 };

	return wd;
}
