#pragma once
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/world/physics/DynamicHitbox.hpp>

class World;
class Inventory;

class ItemOnGround {
	friend class ItemOnGroundManager;
public:
	ItemOnGround(const glm::ivec2& pos, World& world, const Item& item, float lifetime, Hitbox& targetHitbox, Inventory& targetInventory);
	~ItemOnGround();

	ItemOnGround(const ItemOnGround&) = default;
	ItemOnGround& operator=(const ItemOnGround&) = default;

	//Returns whether this item should be destroyed
	bool step(float decay, float angleDeviation);

	void draw(RE::SpriteBatch& spriteBatch) const;
private:
	RE::TexturePtr m_tex;
	Item m_item;
	DynamicHitbox m_hitbox;
	Hitbox* m_targetHitbox;
	Inventory* m_targetInventory;
	float m_lifetime;
};