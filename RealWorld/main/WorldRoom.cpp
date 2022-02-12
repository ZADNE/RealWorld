#include <RealWorld/main/WorldRoom.hpp>

#include <RealWorld/world/physics/position_conversions.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>


#ifdef _DEBUG
const unsigned int FPS_LIMIT = RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND;
#else
const unsigned int FPS_LIMIT = RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND;
#endif // _DEBUG

WorldRoom::WorldRoom(RE::CommandLineArguments args) :
	m_world(),
	m_worldDrawer(window()->getDims()),
	m_player(m_world, RE::SpriteBatch::std(), m_itemOnGroundManager),
	m_inventoryDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont),
	m_craftingDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont),
	m_itemOnGroundManager(RE::SpriteBatch::std(), m_world, m_player.getHitbox(), m_player.getMainInventory()) {

	//Drawers
	m_inventoryDrawer.connectToInventory(&m_player.getMainInventory(), Connection::PRIMARY);
	m_inventoryDrawer.connectToItemUser(&m_player.getItemUser());

	m_craftingDrawer.connectToItemCombinator(&m_player.getItemCombinator());
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

	m_worldView.setPosition(m_player.getCenter());
	synchronizer()->setStepsPerSecond(PHYSICS_STEPS_PER_SECOND);
	synchronizer()->setFramesPerSecondLimit(FPS_LIMIT);
}

void WorldRoom::sessionEnd() {
#ifdef SAVE_WORLD_ON_EXIT
	saveWorld();
#endif // SAVE_WORLD_ON_EXIT
}

void WorldRoom::step() {
	//Player BEGIN
	if (input()->isDown(KB(PLAYER_JUMP))) { m_player.jump(); }
	int walkDir = input()->isDown(KB(PLAYER_LEFT)) ? -1 : 0;
	walkDir += input()->isDown(KB(PLAYER_RIGHT)) ? +1 : 0;
	m_player.walk(static_cast<WALK>(walkDir));
	m_player.step(input()->isDown(RE::Key::LShift));
	//View
	glm::vec2 prevViewPos = m_worldView.getPosition();

	glm::vec2 targetViewPos = m_player.getCenter() * 0.75f + m_worldView.getCursorRel() * 0.25f;

	m_worldView.setCursorAbs(input()->getCursorAbs());
	m_worldView.setPosition(prevViewPos * 0.875f + targetViewPos * 0.125f);
	m_worldViewUnifromBuffer.overwrite(m_worldView.getViewMatrix());

	m_worldDrawer.beginStep(m_worldView.getBotLeft(), m_world);
	//Player END
	m_player.endStep((glm::ivec2)m_worldView.getCursorRel());
	//World BEGIN STEP
	m_world.step();
	auto lm = m_worldDrawer.getLightManipulator();

	//lm.addLight(m_worldView.getCursorRel(), RE::Colour{0u, 0u, 255u, 255u}, 0.0f, 1.0f);

	//ItemOnGroundManager
	m_itemOnGroundManager.step();
	//Inventory and CraftingDrawer
	m_inventoryDrawer.step((glm::ivec2)input()->getCursorAbs());
	m_craftingDrawer.step((glm::ivec2)input()->getCursorAbs());
	if (input()->wasPressed(KB(INV_SWITCHSTATE))) { m_inventoryDrawer.switchState(); m_craftingDrawer.switchDraw(); }
	if (m_inventoryDrawer.isOpen()) {//OPEN INVENTORY
		if (input()->wasPressed(KB(INV_SWAPCURSOR))) { m_inventoryDrawer.swapUnderCursor(); }
		if (input()->wasPressed(KB(INV_MOVEPORTION))) { m_inventoryDrawer.movePortion(0.5f); }

		if (input()->wasPressed(KB(CRAFT_ONE))) { m_craftingDrawer.craft(1u); }
		if (input()->wasPressed(KB(CRAFT_SOME))) { m_craftingDrawer.craft(5u); }
		if (input()->wasPressed(KB(CRAFT_ROLL_RIGHT))) { m_craftingDrawer.roll(input()->wasPressed(KB(CRAFT_ROLL_RIGHT))); }
		if (input()->wasPressed(KB(CRAFT_ROLL_LEFT))) { m_craftingDrawer.roll(-(int)input()->wasPressed(KB(CRAFT_ROLL_LEFT))); }
		if (input()->wasPressed(KB(CRAFT_CANCEL))) { m_craftingDrawer.cancel(); }
	} else { //CLOSED INVENTORY
		if (input()->wasPressed(KB(INV_PREVSLOT))) { m_inventoryDrawer.chooseSlot(Choose::PREV, 0); }
		if (input()->wasPressed(KB(INV_RIGHTSLOT))) { m_inventoryDrawer.chooseSlot(Choose::RIGHT, input()->wasPressed(KB(INV_RIGHTSLOT))); }
		if (input()->wasPressed(KB(INV_LEFTSLOT))) { m_inventoryDrawer.chooseSlot(Choose::LEFT, input()->wasPressed(KB(INV_LEFTSLOT))); }
		if (input()->wasPressed(KB(INV_SLOT0))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 0); }
		if (input()->wasPressed(KB(INV_SLOT1))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 1); }
		if (input()->wasPressed(KB(INV_SLOT2))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 2); }
		if (input()->wasPressed(KB(INV_SLOT3))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 3); }
		if (input()->wasPressed(KB(INV_SLOT4))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 4); }
		if (input()->wasPressed(KB(INV_SLOT5))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 5); }
		if (input()->wasPressed(KB(INV_SLOT6))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 6); }
		if (input()->wasPressed(KB(INV_SLOT7))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 7); }
		if (input()->wasPressed(KB(INV_SLOT8))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 8); }
		if (input()->wasPressed(KB(INV_SLOT9))) { m_inventoryDrawer.chooseSlot(Choose::ABS, 9); }

		if (input()->wasPressed(KB(INV_USEPRIMARY))) { m_player.getItemUser().beginUse(ItemUse::MAIN); }
		if (input()->wasReleased(KB(INV_USEPRIMARY))) { m_player.getItemUser().endUse(ItemUse::MAIN); }
		if (input()->wasPressed(KB(INV_USESECONDARY))) { m_player.getItemUser().beginUse(ItemUse::ALTERNATIVE); }
		if (input()->wasReleased(KB(INV_USESECONDARY))) { m_player.getItemUser().endUse(ItemUse::ALTERNATIVE); }
	}
	m_worldDrawer.endStep();
	//TEMP or DEBUG
	if (input()->wasPressed(KB(DEBUG_WORLDDRAW))) { m_worldDrawer.toggleMinimap(); }
	if (input()->wasPressed(KB(DEBUG_WORLDDARKNESS))) { m_worldDrawer.toggleDarkness(); }
	if (input()->wasPressed(KB(DEBUG_ENDGAME))) { program()->scheduleRoomTransition(0, {}); }
}

void WorldRoom::render(double interpolationFactor) {
	m_worldViewUnifromBuffer.bind();//World view matrix

	m_worldDrawer.drawTiles();

	RE::SpriteBatch::std().begin();
	m_itemOnGroundManager.draw();
	m_player.draw();
	RE::SpriteBatch::std().end(RE::GlyphSortType::TEXTURE);
	RE::SpriteBatch::std().draw();

	m_worldDrawer.coverWithDarkness();

	RE::Viewport::getWindowMatrixUniformBuffer().bind();//Window view matrix

	drawGUI();
}

void WorldRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	window()->resize(newDims, isPermanent);
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

	stream << "Chunks: " << m_world.getNumberOfChunksLoaded() << '\n';

	stream << "Items: " << m_itemOnGroundManager.getNumberOfItemsOG() << '\n';

	glm::ivec2 cursorPosPx = pxToTi(m_player.getCenter());
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

	m_worldDrawer.setTarget(worldTextureSize);
	return true;
}

bool WorldRoom::saveWorld() const {
	WorldData wd;
	m_world.gatherWorldData(wd);
	m_player.gatherPlayerData(wd.pd);
	if (!WorldDataLoader::saveWorldData(wd, wd.wi.worldName)) return false;
	return m_world.saveChunks();
}
