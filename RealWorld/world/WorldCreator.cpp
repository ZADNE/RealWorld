#include <RealWorld/world/WorldCreator.hpp>

#include <RealWorld/metadata.hpp>

WorldData WorldCreator::createWorld(std::string worldName, int seed, glm::uvec2 chunkDims/* = glm::uvec2(128u, 128u)*/) {
	WorldData wd;
	//World info
	wd.wi.seed = seed;
	wd.wi.chunkDims = chunkDims;
	wd.wi.worldName = worldName;

	//Player data
	wd.pd.pos = glm::ivec2(chunkDims) * glm::ivec2(0, 5) * iTILE_SIZE;
	wd.pd.id.resize({10, 4});
	wd.pd.id.items[0][0] = Item{I_ID::STEEL_PICKAXE, 1};
	wd.pd.id.items[1][0] = Item{I_ID::STEEL_HAMMER, 1};

	return wd;
}
