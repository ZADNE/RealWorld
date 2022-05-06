/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vector>

#include <glm/vec2.hpp>

#include <RealWorld/items/Item.hpp>

/**
 * @brief Is a data structure that holds items.
*/
class InventoryData {
	friend class Inventory;
public:
	InventoryData() {}
	InventoryData(glm::ivec2 size) : p_size(size), p_items(slotCount()) { }

	glm::ivec2 getSize() const { return p_size; }


	int toIndex(int x, int y) const {
		return y * p_size.x + x;
	}

	int slotCount() const {
		return p_size.x * p_size.y;
	}

	void resize(const glm::ivec2& newSize) {
		p_size = newSize;
		p_items.resize(slotCount());
	}

	class Proxy {
	public:
		Proxy(InventoryData& inv, int x) : p_inv(inv), p_x(x) { }

		Item& operator[](int y) {
			return p_inv.p_items[p_inv.toIndex(p_x, y)];
		}
	private:
		InventoryData& p_inv;
		int p_x;
	};

	Proxy operator[](int x) {
		return Proxy(*this, x);
	}

	Item& operator()(int i) {
		return p_items[i];
	}

	const Item& operator()(int i) const {
		return p_items[i];
	}

protected:
	glm::ivec2 p_size = glm::ivec2(0, 0);
	std::vector<Item> p_items;
};