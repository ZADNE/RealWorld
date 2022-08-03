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
	InventoryData(glm::ivec2 size) : m_size(size), m_items(slotCount()) { }

	glm::ivec2 getSize() const { return m_size; }


	int toIndex(int x, int y) const {
		return y * m_size.x + x;
	}

	int slotCount() const {
		return m_size.x * m_size.y;
	}

	void resize(const glm::ivec2& newSize) {
		m_size = newSize;
		m_items.resize(slotCount());
	}

	class Proxy {
	public:
		Proxy(InventoryData& inv, int x) : m_inv(inv), m_x(x) { }

		Item& operator[](int y) {
			return m_inv.m_items[m_inv.toIndex(m_x, y)];
		}
	private:
		InventoryData& m_inv;
		int m_x;
	};

	Proxy operator[](int x) {
		return Proxy(*this, x);
	}

	Item& operator()(int i) {
		return m_items[i];
	}

	const Item& operator()(int i) const {
		return m_items[i];
	}

protected:
	glm::ivec2 m_size = glm::ivec2(0, 0);
	std::vector<Item> m_items;
};