#pragma once
#include <string>

#include "physics/PlayerData.hpp"
#include "../furniture/FurnitureCollection.hpp"
#include "WorldInfo.hpp"

struct WorldData {

	std::string path;//Including ending slash
	WorldInfo wi;
	PlayerData pd;
	FurnitureCollection fc;
};