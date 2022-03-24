﻿#include <RealWorld/main/WorldRoom.hpp>

#include <RealWorld/world/physics/position_conversions.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>


WorldRoom::WorldRoom(RE::CommandLineArguments args) :
	m_world(),
	m_worldDrawer(window()->getDims()),
	m_player(RE::SpriteBatch::std()),
	m_playerInventory({10, 4}),
	m_itemOnGroundManager(RE::SpriteBatch::std(), m_world, m_player.getHitbox(), m_playerInventory),
	m_itemUser(m_world, m_playerInventory, m_player.getHitbox(), RE::SpriteBatch::std(), m_itemOnGroundManager),
	m_inventoryDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont),
	m_craftingDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont) {


	m_itemCombinator.connectToInventory(&m_playerInventory);
	m_itemCombinator.connectToIID(&m_instructionDatabase);

	//Drawers
	m_inventoryDrawer.connectToInventory(&m_playerInventory, Connection::PRIMARY);
	m_inventoryDrawer.connectToItemUser(&m_itemUser);

	m_craftingDrawer.connectToItemCombinator(&m_itemCombinator);
}

WorldRoom::~WorldRoom() {

}

void WorldRoom::sessionStart(const RE::RoomTransitionParameters& params) {
	try {
		if (!loadWorld(std::any_cast<const std::string&>(params[0]))) {
			program()->scheduleRoomTransition(0, {});
			return;
		}
	}
	catch (...) {
		RE::fatalError("Bad transition paramaters to start WorldRoom session");
	}

	m_worldView.setPosition(glm::vec2(m_player.getHitbox().getCenter()));
}

void WorldRoom::sessionEnd() {
	//saveWorld();
}

void WorldRoom::step() {
	//Player BEGIN
	int walkDir = input()->isDown(KB[PLAYER_LEFT]) ? -1 : 0;
	walkDir += input()->isDown(KB[PLAYER_RIGHT]) ? +1 : 0;
	m_player.step(static_cast<WALK>(walkDir), input()->isDown(KB[PLAYER_JUMP]), input()->isDown(KB[PLAYER_AUTOJUMP]));
	//View
	glm::vec2 prevViewPos = m_worldView.getPosition();

	glm::vec2 targetViewPos = glm::vec2(m_player.getHitbox().getCenter()) * 0.75f + m_worldView.getCursorRel() * 0.25f;

	m_worldView.setCursorAbs(input()->getCursorAbs());
	m_worldView.setPosition(prevViewPos * 0.875f + targetViewPos * 0.125f);
	m_worldViewUBO.overwrite(0u, m_worldView.getViewMatrix());

	auto viewEnvelope = m_worldDrawer.setPosition(m_worldView.getBotLeft());
	m_world.step(viewEnvelope.botLeftTi, viewEnvelope.topRightTi);
	m_worldDrawer.beginStep();

	bool itemUse[2];
	itemUse[ItemUser::PRIMARY_USE] = input()->isDown(KB[ITEMUSER_USE_PRIMARY]) != 0 && !m_inventoryDrawer.isOpen();
	itemUse[ItemUser::SECONDARY_USE] = input()->isDown(KB[ITEMUSER_USE_SECONDARY]) != 0 && !m_inventoryDrawer.isOpen();
	m_itemUser.step(itemUse, m_worldView.getCursorRel());

	auto lm = m_worldDrawer.getLightManipulator();

	//lm.addLight(m_worldView.getCursorRel(), RE::Colour{0u, 0u, 255u, 255u}, 0.0f, 1.0f);

	//ItemOnGroundManager
	m_itemOnGroundManager.step();
	//Inventory and CraftingDrawer
	m_inventoryDrawer.step((glm::ivec2)input()->getCursorAbs());
	m_craftingDrawer.step((glm::ivec2)input()->getCursorAbs());
	if (input()->wasPressed(KB[INV_SWITCH_STATE])) { m_inventoryDrawer.switchState(); m_craftingDrawer.switchDraw(); }
	if (m_inventoryDrawer.isOpen()) {//OPEN INVENTORY
		if (input()->wasPressed(KB[INV_SWAP_CURSOR])) { m_inventoryDrawer.swapUnderCursor(); }
		if (input()->wasPressed(KB[INV_MOVE_PORTION])) { m_inventoryDrawer.movePortion(0.5f); }

		if (input()->wasPressed(KB[CRAFT_ONE])) { m_craftingDrawer.craft(1u); }
		if (input()->wasPressed(KB[CRAFT_SOME])) { m_craftingDrawer.craft(5u); }
		if (int roll = input()->wasPressed(KB[CRAFT_ROLL_RIGHT])) { m_craftingDrawer.roll(roll); }
		if (int roll = -input()->wasPressed(KB[CRAFT_ROLL_LEFT])) { m_craftingDrawer.roll(roll); }
		if (input()->wasPressed(KB[CRAFT_CANCEL])) { m_craftingDrawer.cancel(); }
	} else { //CLOSED INVENTORY
		if (input()->isDown(KB[ITEMUSER_HOLD_TO_RESIZE])) {
			if (input()->wasPressed(KB[ITEMUSER_WIDEN])) { m_itemUser.resizeShape(0.5f); }
			if (input()->wasPressed(KB[ITEMUSER_SHRINK])) { m_itemUser.resizeShape(-0.5f); }
		} else {
			if (input()->wasPressed(KB[INV_RIGHT_SLOT])) { m_inventoryDrawer.chooseSlot(Choose::RIGHT, input()->wasPressed(KB[INV_RIGHT_SLOT])); }
			if (input()->wasPressed(KB[INV_LEFT_SLOT])) { m_inventoryDrawer.chooseSlot(Choose::LEFT, input()->wasPressed(KB[INV_LEFT_SLOT])); }
		}
		if (input()->wasPressed(KB[INV_PREV_SLOT])) { m_inventoryDrawer.chooseSlot(Choose::PREV, 0); }
		if (input()->wasPressed(KB[INV_SLOT0])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 0); }
		if (input()->wasPressed(KB[INV_SLOT1])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 1); }
		if (input()->wasPressed(KB[INV_SLOT2])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 2); }
		if (input()->wasPressed(KB[INV_SLOT3])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 3); }
		if (input()->wasPressed(KB[INV_SLOT4])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 4); }
		if (input()->wasPressed(KB[INV_SLOT5])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 5); }
		if (input()->wasPressed(KB[INV_SLOT6])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 6); }
		if (input()->wasPressed(KB[INV_SLOT7])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 7); }
		if (input()->wasPressed(KB[INV_SLOT8])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 8); }
		if (input()->wasPressed(KB[INV_SLOT9])) { m_inventoryDrawer.chooseSlot(Choose::ABS, 9); }

		if (input()->wasPressed(KB[ITEMUSER_SWITCH_SHAPE])) { m_itemUser.switchShape(); }
	}
	m_worldDrawer.endStep();
	//TEMP or DEBUG
	if (input()->wasPressed(KB[DEBUG_WORLDDRAW])) { m_worldDrawer.toggleMinimap(); }
	if (input()->wasPressed(KB[DEBUG_WORLDDARKNESS])) { m_worldDrawer.toggleDarkness(); }
	if (input()->wasPressed(KB[DEBUG_ENDGAME])) { program()->scheduleRoomTransition(0, {}); }
}

void WorldRoom::render(double interpolationFactor) {
	m_worldViewUBO.bindIndexed();

	m_worldDrawer.drawTiles();

	RE::SpriteBatch::std().begin();
	m_itemOnGroundManager.draw();
	m_player.draw();
	RE::SpriteBatch::std().end(RE::GlyphSortType::TEXTURE);
	RE::SpriteBatch::std().draw();

	m_worldDrawer.coverWithDarkness();

	RE::Viewport::getWindowMatrixUniformBuffer().bindIndexed();

	drawGUI();
}

void WorldRoom::windowResized(const glm::ivec2& newDims) {
	m_worldView.resizeView(newDims);
	m_worldDrawer.resizeView(newDims);
	m_inventoryDrawer.resizeWindow(newDims);
	m_craftingDrawer.resizeWindow(newDims);
}

void WorldRoom::drawGUI() {
	RE::SpriteBatch::std().begin();
	std::stringstream stream;

	stream << "FPS: " << synchronizer()->getFramesPerSecond() << '\n';
	stream << "Max FT: " << std::chrono::duration_cast<std::chrono::microseconds>(synchronizer()->getMaxFrameTime()).count() << " us" << '\n';
	stream << "RAM chunks: " << m_world.getNumberOfInactiveChunks() << '\n';
	stream << "Items: " << m_itemOnGroundManager.getNumberOfItemsOG() << '\n';
	glm::ivec2 cursorPosPx = pxToTi(m_worldView.getCursorRel());
	stream << "CursorTi: [" << std::setw(4) << cursorPosPx.x << ", "
		<< std::setw(4) << cursorPosPx.y << "]\n";


	glm::vec2 topLeft = glm::vec2(0.0f, window()->getDims().y);
	RE::Colour tint{200, 0, 0, 255};
	auto font = RE::RM::getFont(m_inventoryFont);
	font->add(RE::SpriteBatch::std(), stream.str(), topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);

	m_inventoryDrawer.draw();
	m_craftingDrawer.draw();

	RE::SpriteBatch::std().end(RE::GlyphSortType::POS_TOP);
	RE::SpriteBatch::std().draw();

	m_worldDrawer.drawMinimap();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
	WorldData wd = WorldData();

	if (!WorldDataLoader::loadWorldData(wd, worldName)) return false;

	auto worldTextureSize = m_world.adoptWorldData(wd, worldName, window()->getDims());
	m_player.adoptPlayerData(wd.pd);
	m_playerInventory.adoptInventoryData(wd.pd.id);

	m_worldDrawer.setTarget(worldTextureSize);
	return true;
}

bool WorldRoom::saveWorld() const {
	WorldData wd;
	m_world.gatherWorldData(wd);
	m_player.gatherPlayerData(wd.pd);
	m_playerInventory.gatherInventoryData(wd.pd.id);
	if (!WorldDataLoader::saveWorldData(wd, wd.wi.worldName, false)) return false;
	return m_world.saveChunks();
}
