#include <RealWorld/world/WorldRoom.hpp>

#include <RealEngine/UniformManager.hpp>

#include <RealWorld/world/TDB.hpp>

#include <RealWorld/world/WorldDataLoader.hpp>
#include <RealWorld/items/InventoryDrawer.hpp>
#include <RealWorld/items/CraftingDrawer.hpp>
#include <RealWorld/items/ItemOnGroundManager.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>

WorldRoom::WorldRoom() {

}

WorldRoom::~WorldRoom() {

}

void WorldRoom::E_build(const std::vector<std::string>& buildArgs) {
	//Database initialization
	IDB::init();
	TDB::init();
	FDB::init();

	RE::KeyBinder::std().loadBindings();

	//Constructing objects
	m_player = std::make_unique<Player>();
	m_world = std::make_unique<World>();
	m_worldView = std::make_unique<RE::View>();
	m_inventoryDrawer = std::make_unique<InventoryDrawer>();
	m_craftingDrawer = std::make_unique<CraftingDrawer>();
	m_itemOnGroundManager = std::make_unique<ItemOnGroundManager>();

	//World
	m_world->init(RE::View::std.getViewMatrix(), p_MP->getWindowDim(), &RE::SpriteBatch::std(), m_player.get());

	//Player
	m_player->init(p_MP->IM(), m_world.get(), &RE::SpriteBatch::std(), m_itemOnGroundManager.get());

	//World view
	m_worldView->initView(p_MP->getWindowDim());

	//Drawers
	m_inventoryDrawer->init(&RE::SpriteBatch::std(), p_MP->getWindowDim(), m_inventoryFont);
	m_inventoryDrawer->connectToInventory(m_player->getMainInventory(), Connection::PRIMARY);

	m_inventoryDrawer->connectToItemUser(m_player->getItemUser());

	m_craftingDrawer->init(&RE::SpriteBatch::std(), p_MP->getWindowDim(), m_inventoryFont);
	m_craftingDrawer->connectToItemCombinator(m_player->getItemCombinator());

	//ItemOnGroundManager
	m_itemOnGroundManager->init(&RE::SpriteBatch::std(), m_world.get(), std::make_pair<Hitbox*, Inventory*>(&m_player->getHitbox(), m_player->getMainInventory()));
}

void WorldRoom::E_destroy() {

}

void WorldRoom::E_entry(std::vector<void*> enterPointers) {
	if (enterPointers.size() != 1) { p_MP->exitProgram(1); }
	auto& worldName = *reinterpret_cast<std::string*>(enterPointers[0]);

	bool loaded = m_world->loadWorld(worldName);

	delete enterPointers[0];

	if (!loaded) {
		p_MP->goToRoom(0);
	}

	p_MP->setStepsPerSecond(PHYSICS_STEPS_PER_SECOND);
#ifdef _DEBUG
	p_MP->setFramesPerSecondLimit(150u);
#else
	p_MP->setFramesPerSecondLimit(RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND);
#endif // _DEBUG
}

std::vector<void*> WorldRoom::E_exit() {
	p_leavePointers.clear();

	//m_world->saveWorld();/*TEMP*/

	return p_leavePointers;
}

void WorldRoom::E_step() {
	RE::InputManager* IM = p_MP->IM();
	//Player BEGIN
	m_player->beginStep();
	//View
	m_worldView->setPosition(m_player->getPos(), p_MP->IM()->getCursorAbs());
	m_worldView->update();
	//Player END
	m_player->endStep((glm::ivec2)m_worldView->getCursorRel());
	//World BEGIN STEP
	m_world->beginStep(m_worldView->getPosition(), m_worldView->getBotLeft());
	auto lm = m_world->getLightManipulator();
	lm.dynamicLight_add(m_worldView->getCursorRel(), RE::Colour{0u, 0u, 0u, 255u}, 0.0f, 1.0f);

	//ItemOnGroundManager
	m_itemOnGroundManager->step();
	//Inventory and CraftingDrawer
	m_inventoryDrawer->step((glm::ivec2)IM->getCursorAbs());
	m_craftingDrawer->step((glm::ivec2)IM->getCursorAbs());
	if (IM->wasPressed(KB(INV_SWITCHSTATE))) { m_inventoryDrawer->switchState(); m_craftingDrawer->switchDraw(); }
	if (m_inventoryDrawer->isOpen()) {//OPEN INVENTORY
		if (IM->wasPressed(KB(INV_SWAPCURSOR))) { m_inventoryDrawer->swapUnderCursor(); }
		if (IM->wasPressed(KB(INV_MOVEPORTION))) { m_inventoryDrawer->movePortion(0.5f); }

		if (IM->wasPressed(KB(CRAFT_ONE))) { m_craftingDrawer->craft(1u); }
		if (IM->wasPressed(KB(CRAFT_SOME))) { m_craftingDrawer->craft(5u); }
		if (IM->wasPressed(KB(CRAFT_ROLL_RIGHT))) { m_craftingDrawer->roll(IM->wasPressed(KB(CRAFT_ROLL_RIGHT))); }
		if (IM->wasPressed(KB(CRAFT_ROLL_LEFT))) { m_craftingDrawer->roll(-(int)IM->wasPressed(KB(CRAFT_ROLL_LEFT))); }
		if (IM->wasPressed(KB(CRAFT_CANCEL))) { m_craftingDrawer->cancel(); }
	} else { //CLOSED INVENTORY
		if (IM->wasPressed(KB(INV_PREVSLOT))) { m_inventoryDrawer->chooseSlot(Choose::PREV, 0); }
		if (IM->wasPressed(KB(INV_RIGHTSLOT))) { m_inventoryDrawer->chooseSlot(Choose::RIGHT, IM->wasPressed(KB(INV_RIGHTSLOT))); }
		if (IM->wasPressed(KB(INV_LEFTSLOT))) { m_inventoryDrawer->chooseSlot(Choose::LEFT, IM->wasPressed(KB(INV_LEFTSLOT))); }
		if (IM->wasPressed(KB(INV_SLOT0))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 0); }
		if (IM->wasPressed(KB(INV_SLOT1))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 1); }
		if (IM->wasPressed(KB(INV_SLOT2))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 2); }
		if (IM->wasPressed(KB(INV_SLOT3))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 3); }
		if (IM->wasPressed(KB(INV_SLOT4))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 4); }
		if (IM->wasPressed(KB(INV_SLOT5))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 5); }
		if (IM->wasPressed(KB(INV_SLOT6))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 6); }
		if (IM->wasPressed(KB(INV_SLOT7))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 7); }
		if (IM->wasPressed(KB(INV_SLOT8))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 8); }
		if (IM->wasPressed(KB(INV_SLOT9))) { m_inventoryDrawer->chooseSlot(Choose::ABS, 9); }

		if (IM->wasPressed(KB(INV_USEPRIMARY))) { m_player->getItemUser()->beginUse(ItemUse::MAIN); }
		if (IM->wasPressed(KB(INV_USESECONDARY))) { m_player->getItemUser()->beginUse(ItemUse::ALTERNATIVE); }
		if (IM->wasReleased(KB(INV_USEPRIMARY))) { m_player->getItemUser()->endUse(ItemUse::MAIN); }
		if (IM->wasReleased(KB(INV_USESECONDARY))) { m_player->getItemUser()->endUse(ItemUse::ALTERNATIVE); }
	}
	//World END STEP
	m_world->endStep();
	//TEMP or DEBUG
	if (IM->wasPressed(KB(DEBUG_ENDGAME))) { p_MP->goToRoom(0); }
	if (IM->wasPressed(KB(DEBUG_WORLDDRAW))) { m_world->switchDebugDraw(); }
	if (IM->wasPressed(KB(DEBUG_WORLDDARKNESS))) { m_world->switchDebugDarkness(); }
}

void WorldRoom::E_draw(double interpolationFactor) {
	RE::SpriteBatch::std().begin();
	//World draw NORMAL
	m_world->drawBeginStep();

	//ItemOnGroundManager
	m_itemOnGroundManager->draw();

	RE::SpriteBatch::std().end();
	auto temp = m_worldView->getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &temp);
	RE::SpriteBatch::std().draw();
	//Player draw
	RE::SpriteBatch::std().begin();
	m_player->draw();
	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
	temp = RE::View::std.getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &temp);

	m_world->drawEndStep();

	//GUI
	drawGUI();
}

int WorldRoom::getNextIndex() const {
	return Room::NO_ROOM_INDEX;
}

int WorldRoom::getPrevIndex() const {
	return 0;
}

void WorldRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	p_MP->resizeWindow(newDims, isPermanent);
	m_worldView->resizeView(glm::vec2(newDims));
	m_worldView->update();
	m_world->resizeWindow(RE::View::std.getViewMatrix(), glm::uvec2(newDims));
	m_inventoryDrawer->resizeWindow(glm::vec2(newDims));
	m_craftingDrawer->resizeWindow(glm::vec2(newDims));
}

void WorldRoom::drawGUI() {
	RE::SpriteBatch::std().begin(RE::GlyphSortType::POS_TOP);

	glm::vec2 topLeft = glm::vec2(0.0f, p_MP->getWindowDim().y);
	float row = 0.0f;
	RE::Colour tint{200, 0, 0, 255};

	char buffer[50];

	auto font = RE::RM::getFont(m_inventoryFont);


	//FPS
	snprintf(buffer, 50, "FPS: %i", p_MP->getFramesPerSecond());
	font->add(RE::SpriteBatch::std(), buffer, topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);
	topLeft.y -= font->getFontHeight();
	//MAX FT
	snprintf(buffer, 50, "Max FT: %.lli us", std::chrono::duration_cast<std::chrono::microseconds>(p_MP->getMaxFrameTime()).count());
	font->add(RE::SpriteBatch::std(), buffer, topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);
	topLeft.y -= font->getFontHeight();
	//CHUNKS
	snprintf(buffer, 50, "Chunks: %i", m_world->getNumberOfChunksLoaded());
	font->add(RE::SpriteBatch::std(), buffer, topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);
	topLeft.y -= font->getFontHeight();
	//ITEMS OG
	snprintf(buffer, 50, "ItemsOG: %u", m_itemOnGroundManager->getNumberOfItemsOG());
	font->add(RE::SpriteBatch::std(), buffer, topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);
	topLeft.y -= font->getFontHeight();
	//POSBC
	glm::ivec2 cursorPos = pxToBc(static_cast<glm::ivec2>(m_worldView->getCursorRel()));
	snprintf(buffer, 50, "CursorBc: [%.3i; %.3i]", cursorPos.x, cursorPos.y);
	font->add(RE::SpriteBatch::std(), buffer, topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);
	topLeft.y -= font->getFontHeight();

	m_inventoryDrawer->draw();
	m_craftingDrawer->draw();

	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
}