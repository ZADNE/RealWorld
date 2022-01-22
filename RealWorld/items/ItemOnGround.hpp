#pragma once
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/world/physics/DynamicHitbox.hpp>

class World;
class Inventory;

class ItemOnGround {
	friend class ItemOnGroundManager;
public:
	ItemOnGround(const glm::ivec2& pos, World* world, const Item& item, float lifetime, std::pair<Hitbox*, Inventory*> target);
	~ItemOnGround();

	//Returns whether this item should be destroyed
	bool step(float decay, float angleDeviation);

	void draw(RE::SpriteBatch& spriteBatch) const;
private:
	RE::TexturePtr m_tex;
	Item m_item;
	DynamicHitbox m_hitbox;
	std::pair<Hitbox*, Inventory*> m_target = std::make_pair<Hitbox*, Inventory*>(nullptr, nullptr);
	float m_lifetime;
};