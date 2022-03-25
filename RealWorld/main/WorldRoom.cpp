#include <RealWorld/main/WorldRoom.hpp>

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
	int walkDir = keybindDown(PLAYER_LEFT) ? -1 : 0;
	walkDir += keybindDown(PLAYER_RIGHT) ? +1 : 0;
	m_player.step(static_cast<WALK>(walkDir), keybindDown(PLAYER_JUMP), keybindDown(PLAYER_AUTOJUMP));
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
	itemUse[ItemUser::PRIMARY_USE] = keybindDown(ITEMUSER_USE_PRIMARY) != 0 && !m_inventoryDrawer.isOpen();
	itemUse[ItemUser::SECONDARY_USE] = keybindDown(ITEMUSER_USE_SECONDARY) != 0 && !m_inventoryDrawer.isOpen();
	m_itemUser.step(itemUse, m_worldView.getCursorRel());

	auto lm = m_worldDrawer.getLightManipulator();

	//lm.addLight(m_worldView.getCursorRel(), RE::Colour{0u, 0u, 255u, 255u}, 0.0f, 1.0f);

	//ItemOnGroundManager
	m_itemOnGroundManager.step();
	//Inventory and CraftingDrawer
	m_inventoryDrawer.step((glm::ivec2)input()->getCursorAbs());
	m_craftingDrawer.step((glm::ivec2)input()->getCursorAbs());
	if (keybindPressed(INV_OPEN_CLOSE)) { m_inventoryDrawer.switchState(); m_craftingDrawer.switchDraw(); }
	if (m_inventoryDrawer.isOpen()) {//OPEN INVENTORY
		if (keybindPressed(INV_MOVE_ALL)) { m_inventoryDrawer.swapUnderCursor(); }
		if (keybindPressed(INV_MOVE_PORTION)) { m_inventoryDrawer.movePortion(0.5f); }
	} else { //CLOSED INVENTORY
		if (keybindDown(ITEMUSER_HOLD_TO_RESIZE)) {
			if (keybindPressed(ITEMUSER_WIDEN)) { m_itemUser.resizeShape(0.5f); }
			if (keybindPressed(ITEMUSER_SHRINK)) { m_itemUser.resizeShape(-0.5f); }
		} else {
			if (keybindPressed(INV_RIGHT_SLOT)) { m_inventoryDrawer.chooseSlot(Choose::RIGHT, keybindPressed(INV_RIGHT_SLOT)); }
			if (keybindPressed(INV_LEFT_SLOT)) { m_inventoryDrawer.chooseSlot(Choose::LEFT, keybindPressed(INV_LEFT_SLOT)); }
		}
		if (keybindPressed(INV_PREV_SLOT)) { m_inventoryDrawer.chooseSlot(Choose::PREV, 0); }
		if (keybindPressed(INV_SLOT0)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 0); }
		if (keybindPressed(INV_SLOT1)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 1); }
		if (keybindPressed(INV_SLOT2)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 2); }
		if (keybindPressed(INV_SLOT3)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 3); }
		if (keybindPressed(INV_SLOT4)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 4); }
		if (keybindPressed(INV_SLOT5)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 5); }
		if (keybindPressed(INV_SLOT6)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 6); }
		if (keybindPressed(INV_SLOT7)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 7); }
		if (keybindPressed(INV_SLOT8)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 8); }
		if (keybindPressed(INV_SLOT9)) { m_inventoryDrawer.chooseSlot(Choose::ABS, 9); }

		if (keybindPressed(ITEMUSER_SWITCH_SHAPE)) { m_itemUser.switchShape(); }
	}
	m_worldDrawer.endStep();
	//TEMP or DEBUG
	if (keybindPressed(MINIMAP)) { m_worldDrawer.toggleMinimap(); }
	if (keybindPressed(SHADOWS)) { m_worldDrawer.toggleDarkness(); }
	if (keybindPressed(QUIT)) { program()->scheduleRoomTransition(0, {}); }
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
