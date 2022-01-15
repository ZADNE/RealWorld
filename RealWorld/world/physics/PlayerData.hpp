#pragma once

#include <glm/vec2.hpp>
#include "../../items/InventoryData.hpp"

struct PlayerData {
	glm::ivec2 pos;
	InventoryData id;
};
