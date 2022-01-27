#include <RealWorld/main/WorldRoom.hpp>

#include <RealEngine/graphics/UniformManager.hpp>

#include <RealWorld/world/physics/position_conversions.hpp>

WorldRoom::WorldRoom(RE::CommandLineArguments args) :
	m_world(RE::View::std.getViewMatrix(), window()->getDims(), RE::SpriteBatch::std(), m_player),
	m_player(*input(), m_world, RE::SpriteBatch::std(), m_itemOnGroundManager),
	m_inventoryDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont),
	m_craftingDrawer(RE::SpriteBatch::std(), window()->getDims(), m_inventoryFont),
	m_itemOnGroundManager(RE::SpriteBatch::std(), m_world, m_player.getHitbox(), m_player.getMainInventory()) {

	//World view
	m_worldView.initView(window()->getDims());

	//Drawers
	m_inventoryDrawer.connectToInventory(&m_player.getMainInventory(), Connection::PRIMARY);
	m_inventoryDrawer.connectToItemUser(&m_player.getItemUser());

	m_craftingDrawer.connectToItemCombinator(&m_player.getItemCombinator());
}

WorldRoom::~WorldRoom() {

}

void WorldRoom::sessionStart(const RE::RoomTransitionParameters& params) {
	try {
		if (!m_world.loadWorld(std::any_cast<const std::string&>(params[0]))) {
			program()->scheduleRoomTransition(0, {});
			return;
		}
	}
	catch (...) {
		RE::fatalError("Bad transition paramaters to start WorldRoom session");
	}

	synchronizer()->setStepsPerSecond(PHYSICS_STEPS_PER_SECOND);
#ifdef _DEBUG
	synchronizer()->setFramesPerSecondLimit(150u);
#else
	synchronizer()->setFramesPerSecondLimit(RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND);
#endif // _DEBUG
}

void WorldRoom::sessionEnd() {
	//m_world.saveWorld();/*TEMP*/
}

void WorldRoom::step() {
	//Player BEGIN
	m_player.beginStep();
	//View
	m_worldView.setPosition(m_player.getPos(), input()->getCursorAbs());
	m_worldView.update();
	//Player END
	m_player.endStep((glm::ivec2)m_worldView.getCursorRel());
	//World BEGIN STEP
	m_world.beginStep(m_worldView.getPosition(), m_worldView.getBotLeft());
	auto lm = m_world.getLightManipulator();
	lm.addLight(m_worldView.getCursorRel(), RE::Colour{0u, 0u, 0u, 255u}, 0.0f, 1.0f);

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
		if (input()->wasPressed(KB(INV_USESECONDARY))) { m_player.getItemUser().beginUse(ItemUse::ALTERNATIVE); }
		if (input()->wasReleased(KB(INV_USEPRIMARY))) { m_player.getItemUser().endUse(ItemUse::MAIN); }
		if (input()->wasReleased(KB(INV_USESECONDARY))) { m_player.getItemUser().endUse(ItemUse::ALTERNATIVE); }
	}
	//World END STEP
	m_world.endStep();
	//TEMP or DEBUG
	if (input()->wasPressed(KB(DEBUG_ENDGAME))) { program()->scheduleRoomTransition(0, {}); }
	if (input()->wasPressed(KB(DEBUG_WORLDDRAW))) { m_world.switchDebugDraw(); }
	if (input()->wasPressed(KB(DEBUG_WORLDDARKNESS))) { m_world.switchDebugDarkness(); }
}

void WorldRoom::render(double interpolationFactor) {
	RE::SpriteBatch::std().begin();
	//World draw NORMAL
	m_world.drawBeginStep();

	//ItemOnGroundManager
	m_itemOnGroundManager.draw();

	RE::SpriteBatch::std().end();
	auto temp = m_worldView.getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &temp);
	RE::SpriteBatch::std().draw();
	//Player draw
	RE::SpriteBatch::std().begin();
	m_player.draw();
	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
	temp = RE::View::std.getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &temp);

	m_world.drawEndStep();

	//GUI
	drawGUI();
}

void WorldRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	window()->resize(newDims, isPermanent);
	m_worldView.resizeView(glm::vec2(newDims));
	m_worldView.update();
	m_world.resizeWindow(RE::View::std.getViewMatrix(), glm::uvec2(newDims));
	m_inventoryDrawer.resizeWindow(glm::vec2(newDims));
	m_craftingDrawer.resizeWindow(glm::vec2(newDims));
}

void WorldRoom::drawGUI() {
	RE::SpriteBatch::std().begin(RE::GlyphSortType::POS_TOP);
	std::stringstream stream;

	stream << "FPS: " << synchronizer()->getFramesPerSecond() << '\n';

	stream << "Max FT: " << std::chrono::duration_cast<std::chrono::microseconds>(synchronizer()->getMaxFrameTime()).count() << " us" << '\n';

	stream << "Chunks: " << m_world.getNumberOfChunksLoaded() << '\n';

	stream << "Items: " << m_itemOnGroundManager.getNumberOfItemsOG() << '\n';

	glm::ivec2 cursorPos = pxToBc(static_cast<glm::ivec2>(m_worldView.getCursorRel()));
	stream << "CursorBc: [" << std::setw(4) << cursorPos.x << ", "
		<< std::setw(4) << cursorPos.y << "]" << '\n';


	glm::vec2 topLeft = glm::vec2(0.0f, window()->getDims().y);
	RE::Colour tint{200, 0, 0, 255};
	auto font = RE::RM::getFont(m_inventoryFont);
	font->add(RE::SpriteBatch::std(), stream.str(), topLeft, 0, tint, RE::HAlign::RIGHT, RE::VAlign::BELOW);

	m_inventoryDrawer.draw();
	m_craftingDrawer.draw();

	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
}