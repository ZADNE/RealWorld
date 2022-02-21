#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/items/IDB.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemOnGroundManager.hpp>

class ItemUser {
public:
	const static int PRIMARY_USE = 0;
	const static int SECONDARY_USE = 1;

	ItemUser(World& world, Inventory& inventory, Hitbox& operatorsHitbox, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager);
	~ItemUser();

	void switchShape();
	void resizeShape(float change);

	//Does not check if the slot is inside the inventory!
	void chooseSlot(int slot);

	void step(bool use[2], const glm::ivec2& relCursorPosPx);
	void draw();
private:
	World& m_world;
	Inventory& m_inv;
	Hitbox& m_operatorsHitbox;
	ItemOnGroundManager& m_itemOnGroundManager;
	RE::SpriteBatch& m_spriteBatch;

	int m_chosenSlot = 0;

	SET_SHAPE m_shape = SET_SHAPE::DISC;
	float m_diameter = 1.0f;

	// <  -1: steps not using
	// == -1: just stopped using
	// == +0: invalid state
	// >  +1: steps using
	// == +1: just started using
	int m_using[2] = {-1, -1};
	Item* m_item = nullptr;

	RE::TexturePtr m_miningBlockTex = RE::RM::getTexture("miningBlock");
};
