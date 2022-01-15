#pragma once
#include <RealEngine/Sprite.hpp>

#include "../items/IDB.hpp"

struct Item;

class ItemSprite : public RE::FullSprite {
public:
	ItemSprite();
	ItemSprite(I_ID ID);
	ItemSprite(Item item);
};
