#include <RealWorld/world/LightManipulator.hpp>

#include <RealWorld/world/WorldDrawer.hpp>

DestroyTicket<StaticLight> LightManipulator::staticLight_add(const StaticLight& staticLight) {
	return m_wd->staticLight_add(staticLight);
}

void LightManipulator::staticLight_remove(DestroyTicket<StaticLight> ticket) {
	m_wd->staticLight_remove(ticket);
}

void LightManipulator::staticLight_reserve(size_t add) {
	m_wd->staticLight_reserve(add);
}

void LightManipulator::dynamicLight_add(const DynamicLight& dynamicLight) {
	m_wd->dynamicLight_add(dynamicLight);
}

void LightManipulator::dynamicLight_add(const glm::vec2& posPx, RE::Colour col, float dir, float cone) {
	m_wd->dynamicLight_add(posPx, col, dir, cone);
}
