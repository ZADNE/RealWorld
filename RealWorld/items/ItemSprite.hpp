#pragma once
#include <RealEngine/graphics/Sprite.hpp>

#include <RealWorld/items/IDB.hpp>

struct Item;

class ItemSprite : public RE::FullSprite {
public:
	ItemSprite();
	ItemSprite(I_ID ID);
	ItemSprite(Item item);
};
