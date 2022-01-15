#pragma once
#include <set>
#include <vector>

const size_t NODE_MAX_OBJECT_COUNT = 16;

template<typename T>class Quadtree {
public:
	Quadtree() {}
	Quadtree(size_t level, const T& bounds): m_level(level), m_bounds(bounds) {

	};

	~Quadtree() {
		clear();
	};

	void clear() {
		delete[] m_nodes;
		m_nodes = nullptr;
	};

	void split() {
		if (((m_bounds.dim.x % 2) != 0) || ((m_bounds.dim.y % 2) != 0)) {
			return;
		}
		auto newDims = m_bounds.dim / 2;
		auto BL = m_bounds.botLeft;

		//		|
		//	1	|	0
		//------+------
		//	2	|	3
		//		|
		m_nodes = new Quadtree[4];

		m_nodes[0] = Quadtree(m_level + 1, T({ BL.x + newDims.x, BL.y + newDims.y }, newDims));
		m_nodes[1] = Quadtree(m_level + 1, T({ BL.x, BL.y + newDims.y }, newDims));
		m_nodes[2] = Quadtree(m_level + 1, T({ BL.x, BL.y }, newDims));
		m_nodes[3] = Quadtree(m_level + 1, T({ BL.x + newDims.x, BL.y }, newDims));

		//Moving objects into the child nodes (if possible)
		for (auto it = m_objects.begin(); it != m_objects.end();) {
			int index = getIndex(*it);
			if (index != -1) {
				m_nodes[index].insert(*it);
				m_objects.erase(it++);
			} else {
				++it;
			}
		}
	};

	void insert(const T& object) {
		if (m_nodes != nullptr) {
			//Node is further divided
			int index = getIndex(object);
			//Object does fit one of the sub nodes => we insert it there
			if (index != -1) {
				m_nodes[index].insert(object);
				return;
			}
		}
		//This is a leaf node
		m_objects.insert(object);

		if (m_objects.size() >= NODE_MAX_OBJECT_COUNT) {
			split();
		}
	};

	void retrieve(std::vector<T>& possibleCollisions, const T& object) const {
		int index = getIndex(object);
		if (m_nodes != nullptr) {
			if (index != -1) {//object fits inside one child node
				m_nodes[index].retrieve(possibleCollisions, object);
			} else {//object does not fit inside one child node -> all have to be included
				m_nodes[0].retrieve(possibleCollisions, object);
				m_nodes[1].retrieve(possibleCollisions, object);
				m_nodes[2].retrieve(possibleCollisions, object);
				m_nodes[3].retrieve(possibleCollisions, object);
			}
		}
		possibleCollisions.reserve(possibleCollisions.size() + m_objects.size());
		for (auto& object : m_objects) {
			possibleCollisions.push_back(object);
		}
	};

	bool empty() const {
		if (m_nodes != nullptr) {
			for (size_t i = 0u; i < 4u; ++i) {
				if (!m_nodes[i].empty()) {
					return false;
				}
			}
		}
		if (!m_objects.empty()) {
			return false;
		}
		return true;
	}

	//Erases all objects that overlap 'object'
	size_t erase(const T& object) {
		int index = getIndex(object);
		size_t removed = 0u;
		if (index != -1 && m_nodes != nullptr) {
			removed += m_nodes[index].erase(object);
		}
		for (auto it = m_objects.begin(); it != m_objects.end();) {
			if (it->collidesWith(object)) {
				m_objects.erase(it++);
				++removed;
			} else {
				++it;
			}
		}
		return removed;
	}
private:
	//Determines which node the object belongs to.
	//-1 means that object cannot completely fit within any child node
	//and thus should be part of the parent node.
	//0, 1, 2, 3 = mathematical quadrants

	//		|
	//	1	|	0
	//------+------
	//	2	|	3
	//		|
	int getIndex(const T& object) const {
		int index = -1;
		auto middle = m_bounds.botLeft + m_bounds.dim / 2;

		//object can completely fit within the top quadrants
		bool topQuadrant = (object.botLeft.y > middle.y);
		//object can completely fit within the bottom quadrants
		bool bottomQuadrant = (object.botLeft.y + object.dim.y < middle.y);

		if (object.botLeft.x + object.dim.x < middle.x) {
			//object can completely fit within the left quadrants
			if (topQuadrant) {
				index = 1;
			} else if (bottomQuadrant) {
				index = 2;
			}
		} else if (object.botLeft.x > middle.x) {
			//object can completely fit within the right quadrants
			if (topQuadrant) {
				index = 0;
			} else if (bottomQuadrant) {
				index = 3;
			}
		}

		return index;
	};

	T m_bounds;
	size_t m_level;

	Quadtree* m_nodes = nullptr;
	std::set<T> m_objects;
};