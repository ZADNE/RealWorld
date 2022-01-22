#pragma once
#include <string>

#include <RealWorld/world/physics/PlayerData.hpp>
#include <RealWorld/furniture/FurnitureCollection.hpp>
#include <RealWorld/world/WorldInfo.hpp>

struct WorldData {

	std::string path;//Including ending slash
	WorldInfo wi;
	PlayerData pd;
	FurnitureCollection fc;
};