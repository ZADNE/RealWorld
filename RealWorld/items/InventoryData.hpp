#pragma once
#include <vector>

#include <glm/vec2.hpp>

#include <RealWorld/items/Item.hpp>

struct InventoryData {
	std::vector<std::vector<Item>> items;
	glm::ivec2 size = glm::ivec2(0, 0);

	void resize(const glm::ivec2& newSize) {
		size = newSize;
		items.resize(size.x);
		for (int x = 0; x < size.x; ++x) {
			items[x].resize(size.y);
		}
	}
};