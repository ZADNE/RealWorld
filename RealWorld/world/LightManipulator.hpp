#pragma once
#include <RealEngine/graphics/Vertex.hpp>

#include <RealWorld/world/DynamicLight.hpp>

class WorldDrawer;

class LightManipulator {
	friend class WorldDrawer;
public:
	LightManipulator() : m_wd(nullptr) {}

	void addLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone);
private:
	LightManipulator(WorldDrawer* wd) : m_wd(wd) {};
	WorldDrawer* m_wd;
};