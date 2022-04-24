﻿#include <RealWorld/main/WorldRoom.hpp>

#include <RealWorld/save/WorldSaveLoader.hpp>

WorldRoom::WorldRoom(RE::CommandLineArguments args) :
	m_world(),
	m_worldDrawer(window()->getDims()),
	m_player(RE::SpriteBatch::std()),
	m_playerInv({10, 4}),
	m_itemUser(m_world, m_playerInv, m_player.getHitbox(), RE::SpriteBatch::std()),
	m_invUI(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont) {

	//Drawers
	m_invUI.connectToInventory(&m_playerInv, InventoryUI::Connection::PRIMARY);
	m_invUI.connectToItemUser(&m_itemUser);
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
	using enum InventoryUI::SelectionManner;

	//Player
	int walkDir = keybindDown(PLAYER_LEFT) ? -1 : 0;
	walkDir += keybindDown(PLAYER_RIGHT) ? +1 : 0;
	m_player.step(static_cast<WALK>(walkDir), keybindDown(PLAYER_JUMP), keybindDown(PLAYER_AUTOJUMP));

	//View
	glm::vec2 prevViewPos = m_worldView.getPosition();
	glm::vec2 targetViewPos = glm::vec2(m_player.getHitbox().getCenter()) * 0.75f + m_worldView.getCursorRel() * 0.25f;
	m_worldView.setCursorAbs(input()->getCursorAbs());
	m_worldView.setPosition(prevViewPos * 0.875f + targetViewPos * 0.125f);
	m_worldViewUBO.overwrite(0u, m_worldView.getViewMatrix());

	//World
	auto viewEnvelope = m_worldDrawer.setPosition(m_worldView.getBotLeft());
	m_world.step(viewEnvelope.botLeftTi, viewEnvelope.topRightTi);
	m_worldDrawer.beginStep();

	//Item user
	m_itemUser.step(
		keybindDown(ITEMUSER_USE_PRIMARY) && !m_invUI.isOpen(),
		keybindDown(ITEMUSER_USE_SECONDARY) && !m_invUI.isOpen(),
		m_worldView.getCursorRel()
	);

	m_worldDrawer.addLight(m_worldView.getCursorRel(), RE::Color{0u, 0u, 0u, 31u});

	//Inventory
	m_invUI.step();
	if (keybindPressed(INV_OPEN_CLOSE)) { m_invUI.openOrClose(); }
	if (m_invUI.isOpen()) {//Inventory is open
		if (keybindPressed(INV_MOVE_ALL)) { m_invUI.swapUnderCursor(input()->getCursorAbs()); }
		if (keybindPressed(INV_MOVE_PORTION)) { m_invUI.movePortion(input()->getCursorAbs(), 0.5f); }
	} else { //Inventory is closed
		if (keybindDown(ITEMUSER_HOLD_TO_RESIZE)) {
			if (keybindPressed(ITEMUSER_WIDEN)) { m_itemUser.resizeShape(0.5f); }
			if (keybindPressed(ITEMUSER_SHRINK)) { m_itemUser.resizeShape(-0.5f); }
		} else {
			if (keybindPressed(INV_RIGHT_SLOT)) { m_invUI.selectSlot(RIGHT, keybindPressed(INV_RIGHT_SLOT)); }
			if (keybindPressed(INV_LEFT_SLOT)) { m_invUI.selectSlot(LEFT, keybindPressed(INV_LEFT_SLOT)); }
		}
		if (keybindPressed(INV_PREV_SLOT)) { m_invUI.selectSlot(PREV, 0); }

		constexpr int SLOT0_INT = static_cast<int>(INV_SLOT0);
		for (int i = 0; i < 10; ++i) {
			if (keybindPressed(static_cast<RealWorldKeyBindings>(SLOT0_INT + i))) { m_invUI.selectSlot(ABS, i); }
		}
		if (keybindPressed(ITEMUSER_SWITCH_SHAPE)) { m_itemUser.switchShape(); }
	}
	m_worldDrawer.endStep();

	//Toggles & quit
	if (keybindPressed(MINIMAP)) { m_worldDrawer.toggleMinimap(); }
	if (keybindPressed(SHADOWS)) { m_worldDrawer.toggleShadows(); }
	if (keybindPressed(QUIT)) { program()->scheduleRoomTransition(0, {}); }
}

void WorldRoom::render(double interpolationFactor) {
	m_worldViewUBO.bindIndexed();

	m_worldDrawer.drawTiles();

	RE::SpriteBatch::std().begin();
	m_player.draw();
	RE::SpriteBatch::std().end(RE::GlyphSortType::TEXTURE);
	RE::SpriteBatch::std().draw();

	m_worldDrawer.coverWithShadows();

	RE::Viewport::getWindowMatrixUniformBuffer().bindIndexed();

	drawGUI();
}

void WorldRoom::windowResized(const glm::ivec2& newDims) {
	m_worldView.resizeView(newDims);
	m_worldDrawer.resizeView(newDims);
	m_invUI.windowResized(newDims);
}

void WorldRoom::drawGUI() {
	RE::SpriteBatch::std().begin();
	std::stringstream stream;

	stream << "FPS: " << synchronizer()->getFramesPerSecond() << '\n';
	stream << "Max FT: " << std::chrono::duration_cast<std::chrono::microseconds>(synchronizer()->getMaxFrameTime()).count() << " us" << '\n';
	stream << "CPU chunks: " << m_world.getNumberOfInactiveChunks() << '\n';
	glm::ivec2 cursorPosPx = pxToTi(m_worldView.getCursorRel());
	stream << "CursorTi: [" << std::setw(4) << cursorPosPx.x << ", "
		<< std::setw(4) << cursorPosPx.y << "]\n";

	glm::vec2 topLeft = glm::vec2(0.0f, window()->getDims().y);
	RE::RM::getFont(m_inventoryFont)->add(RE::SpriteBatch::std(), stream.str(), topLeft, 0, RE::Color{255, 255, 255, 255}, RE::HAlign::RIGHT, RE::VAlign::BELOW);

	m_invUI.draw(input()->getCursorAbs());

	RE::SpriteBatch::std().end(RE::GlyphSortType::POS_TOP);
	RE::SpriteBatch::std().draw();

	m_worldDrawer.drawMinimap();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
	WorldSave save{};

	if (!WorldSaveLoader::loadWorld(save, worldName)) return false;

	auto worldTextureSize = m_world.adoptSave(save.metadata, window()->getDims());
	m_player.adoptSave(save.player);
	m_playerInv.adoptInventoryData(save.inventory);

	m_worldDrawer.setTarget(worldTextureSize);
	return true;
}

bool WorldRoom::saveWorld() const {
	WorldSave save{};
	m_world.gatherSave(save.metadata);
	m_player.gatherSave(save.player);
	m_playerInv.gatherInventoryData(save.inventory);
	if (!WorldSaveLoader::saveWorld(save, save.metadata.worldName, false)) return false;
	return m_world.saveChunks();
}
