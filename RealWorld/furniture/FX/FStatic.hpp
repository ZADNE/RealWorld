#pragma once

#include <glm/vec2.hpp>

#include <RealEngine/graphics/Sprite.hpp>

#include <RealWorld/furniture/FDB.hpp>
#include <RealWorld/world/LightManipulator.hpp>

namespace RE {
	class SpriteBatch;
}

class FStatic{
public:
	FStatic(const glm::ivec2& botLeftBc, size_t totalIndex);//Quick and safe (as long as totalIndex is not above bounds)
	FStatic(const glm::ivec2& botLeftBc, F_TYPE type, size_t specificIndex);//Slow (linear search included); may fail (if specificIndex not found)
	~FStatic();

	virtual void build();

	virtual void step();

	virtual void draw();

	virtual void destroy();

	static void initStatics(RE::SpriteBatch* spriteBatch, LightManipulator lightManipulator);


	//Getters
	virtual size_t getTotalIndex() const { return p_totalIndex; };
	virtual F_TYPE getType() const { return FDB::getType(p_totalIndex); };
	virtual uint8_t getSpecificIndex() const { return FDB::getSpecificIndex(p_totalIndex); };
	virtual glm::ivec2 getBotLeft() const { return p_botLeftBc; };
	virtual glm::ivec2 getMiddlePos() const { return p_botLeftBc + getDims() / 2; };
	virtual glm::ivec2 getDims() const { return FDB::getDims(p_totalIndex); };

protected:
	static RE::SpriteBatch* p_spriteBatch;
	static LightManipulator p_lightManipulator;

	glm::ivec2 p_botLeftBc;
	size_t p_totalIndex;

	RE::SpeedSprite p_sprite;
};