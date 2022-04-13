#include <RealWorld/world/LightManipulator.hpp>

#include <RealWorld/world/WorldDrawer.hpp>

void LightManipulator::addLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone) {
	m_wd->addLight(posPx, col, dir, cone);
}
