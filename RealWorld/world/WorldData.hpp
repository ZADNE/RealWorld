#pragma once
#include <string>

#include <RealWorld/physics/PlayerData.hpp>
#include <RealWorld/world/WorldInfo.hpp>

struct WorldData {
	std::string path;//Including ending slash
	WorldInfo wi;
	PlayerData pd;
};