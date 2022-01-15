#pragma once

#include "WorldData.hpp"

class WorldCreator{
public:
	static WorldData createWorld(std::string worldName, int seed, glm::uvec2 chunkDims = glm::uvec2(128u, 128u));

private:

};

