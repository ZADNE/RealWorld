/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

#include <RealWorld/save/WorldSaveLoader.hpp>

WorldRoom::WorldRoom(const GameSettings& gameSettings) :
	Room(1, DEFAULT_SETTINGS),
	m_gameSettings(gameSettings),
	m_world(m_chunkGen),
	m_worldDrawer(engine().getWindowDims()),
	m_player(RE::SpriteBatch::std()),
	m_playerInv({10, 4}),
	m_itemUser(m_world, m_playerInv, m_player.getHitbox()),
	m_invUI(RE::SpriteBatch::std(), engine().getWindowDims()) {

	//InventoryUI connections
	m_invUI.connectToInventory(&m_playerInv, InventoryUI::Connection::PRIMARY);
	m_invUI.connectToItemUser(&m_itemUser);
}

void WorldRoom::sessionStart(const RE::RoomTransitionParameters& params) {
	try {
		const std::string& worldName = std::any_cast<const std::string&>(params[0]);
		if (!loadWorld(worldName)) {
			engine().scheduleRoomTransition(0, {});
			return;
		}
		engine().setWindowTitle("RealWorld! - " + worldName);
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
	RE::GeometryBatch::std().begin();
	using enum InventoryUI::SelectionManner;

	//Player
	int walkDir = keybindDown(PLAYER_LEFT) ? -1 : 0;
	walkDir += keybindDown(PLAYER_RIGHT) ? +1 : 0;
	m_player.step(static_cast<WALK>(walkDir), keybindDown(PLAYER_JUMP), keybindDown(PLAYER_AUTOJUMP));

	//View
	glm::vec2 prevViewPos = m_worldView.getPosition();
	glm::vec2 targetViewPos = glm::vec2(m_player.getHitbox().getCenter()) * 0.75f + m_worldView.getCursorRel() * 0.25f;
	auto viewPos = prevViewPos * 0.875f + targetViewPos * 0.125f;
	//auto viewPos = prevViewPos + glm::vec2(glm::ivec2(engine().getCursorAbs()) - engine().getWindowDims() / 2) * 0.03f;
	m_worldView.setCursorAbs(engine().getCursorAbs());
	m_worldView.setPosition(glm::floor(viewPos));
	m_worldViewUBO.overwrite(0u, m_worldView.getViewMatrix());

	//World
	auto viewEnvelope = m_worldDrawer.setPosition(m_worldView.getBotLeft());
	m_world.step(viewEnvelope.botLeftTi, viewEnvelope.topRightTi);
	m_worldDrawer.beginStep();

	//Item user
	m_itemUser.step(
		keybindDown(ITEMUSER_USE_PRIMARY) && !m_invUI.isOpen(),
		keybindDown(ITEMUSER_USE_SECONDARY) && !m_invUI.isOpen(),
		m_worldView.getCursorRel(), RE::GeometryBatch::std()
	);

	static float rad = 0.0f;
	rad += 0.01f;
	m_worldDrawer.addExternalLight(m_worldView.getCursorRel() + glm::vec2(glm::cos(rad), glm::sin(rad)) * 0.0f, RE::Color{0u, 0u, 0u, 255u});
	m_worldDrawer.addExternalLight(m_player.getHitbox().getCenter(), RE::Color{0u, 0u, 0u, 100u});

	//Inventory
	m_invUI.step();
	if (keybindPressed(INV_OPEN_CLOSE)) { m_invUI.openOrClose(); }
	if (m_invUI.isOpen()) {//Inventory is open
		if (keybindPressed(INV_MOVE_ALL)) { m_invUI.swapUnderCursor(engine().getCursorAbs()); }
		if (keybindPressed(INV_MOVE_PORTION)) { m_invUI.movePortion(engine().getCursorAbs(), 0.5f); }
	} else { //Inventory is closed
		if (keybindDown(ITEMUSER_HOLD_TO_RESIZE)) {
			if (keybindPressed(ITEMUSER_WIDEN)) { m_itemUser.resizeShape(1.0f); }
			if (keybindPressed(ITEMUSER_SHRINK)) { m_itemUser.resizeShape(-1.0f); }
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
	if (keybindPressed(QUIT)) { engine().scheduleRoomTransition(0, {}); }
	if (keybindPressed(MINIMAP)) { m_worldDrawer.shouldDrawMinimap(m_minimap = !m_minimap); }
	if (keybindPressed(SHADOWS)) { m_worldDrawer.shouldDrawShadows(m_shadows = !m_shadows); }
	if (keybindPressed(PERMUTE)) { m_world.shouldPermuteOrder(m_permute = !m_permute); }

	RE::GeometryBatch::std().end();
}

void WorldRoom::render(double interpolationFactor) {
	m_worldViewUBO.bindIndexed();

	m_worldDrawer.drawTiles();

	auto& sb = RE::SpriteBatch::std();
	sb.begin();
	m_player.draw();
	sb.end(RE::GlyphSortType::TEXTURE);
	sb.draw();

	m_worldDrawer.drawShadows();

	RE::GeometryBatch::std().draw();

	RE::Viewport::getWindowMatrixUniformBuffer().bindIndexed();

	drawGUI();
}

void WorldRoom::windowResizedCallback(const glm::ivec2& oldSize, const glm::ivec2& newSize) {
	m_worldView.resizeView(newSize);
	m_worldDrawer.resizeView(newSize);
	m_invUI.windowResized(newSize);
}

void WorldRoom::drawGUI() {
	//Inventory
	RE::SpriteBatch::std().begin();
	m_invUI.draw(engine().getCursorAbs());
	RE::SpriteBatch::std().end(RE::GlyphSortType::POS_TOP);
	RE::SpriteBatch::std().draw();
	//Minimap
	m_worldDrawer.drawMinimap();
	//Top-left menu
	ImGui::SetNextWindowPos({0.0f, 0.0f});
	ImGui::PushFont(m_arial);
	if (ImGui::Begin("##topLeftMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
		ImGui::Text("FPS: %u\nMax FT: %i us",
			engine().getFramesPerSecond(),
			(int)std::chrono::duration_cast<std::chrono::microseconds>(engine().getMaxFrameTime()).count());
		ImGui::Separator();
		ImGui::TextUnformatted("Minimap:"); ImGui::SameLine();
		if (ImGui::ToggleButton("##minimap", &m_minimap)) m_worldDrawer.shouldDrawMinimap(m_minimap);
		ImGui::TextUnformatted("Shadows:"); ImGui::SameLine();
		if (ImGui::ToggleButton("##shadows", &m_shadows)) m_worldDrawer.shouldDrawShadows(m_shadows);
		ImGui::TextUnformatted("Permute:"); ImGui::SameLine();
		if (ImGui::ToggleButton("##shadows", &m_permute)) m_world.shouldPermuteOrder(m_permute);
	}
	ImGui::End();
	ImGui::PopFont();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
	WorldSave save{};

	if (!WorldSaveLoader::loadWorld(save, worldName)) return false;

	m_world.adoptSave(save.metadata, m_gameSettings.getActiveChunksArea());
	m_player.adoptSave(save.player);
	m_playerInv.adoptInventoryData(save.inventory);

	m_worldDrawer.setTarget(m_gameSettings.getActiveChunksArea() * iCHUNK_SIZE);
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
