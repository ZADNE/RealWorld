#pragma once
#include <string>

#include <glm/vec2.hpp>

#include <RealWorld/items/InventoryData.hpp>

struct MetadataSave {
	std::string path;//Including ending slash
	int seed;
	std::string worldName;
};

struct PlayerSave {
	glm::ivec2 pos;
};

struct WorldSave {
	MetadataSave metadata;
	PlayerSave player;
	InventoryData inventory;
};