#pragma once
#include <RealEngine/graphics/Vertex.hpp>

#include <RealWorld/world/DestroyTicket.hpp>
#include <RealWorld/world/StaticLight.hpp>
#include <RealWorld/world/DynamicLight.hpp>

class WorldDrawer;

class LightManipulator {
	friend class WorldDrawer;
public:
	LightManipulator() : m_wd(nullptr) {};

		//Returns ticket used for removing the light
	DestroyTicket<StaticLight> staticLight_add(const StaticLight& staticLight);
		//Ticket is obtained from addStaticLight()
		//Invalid ticket are silently ingnored
	void staticLight_remove(DestroyTicket<StaticLight> ticket);
		//Reserve space for future lights
		//Does not ever have to be called, use before adding large amount of lights
		//add = amount of lightspaces to add to the current number
		//Cannot remove or alter resident lights
	void staticLight_reserve(size_t add);

		//Unlike static lights, dynamic lights must be added each step between beginStep() and endStep()
	void dynamicLight_add(const DynamicLight& dynamicLight);
	void dynamicLight_add(const glm::vec2& posPx, RE::Colour col, GLfloat dir, GLfloat cone);
private:
	LightManipulator(WorldDrawer* wd) : m_wd(wd) {};
	WorldDrawer* m_wd;
};