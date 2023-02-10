/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/save/WorldSaveLoader.hpp>

#ifdef _DEBUG
constexpr unsigned int FPS_LIMIT = 300u;
#else
constexpr unsigned int FPS_LIMIT = RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND;
#endif // _DEBUG

constexpr glm::vec4 SKY_BLUE = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

constexpr RE::RoomDisplaySettings INITIAL_SETTINGS{
    .clearColor = SKY_BLUE,
    .stepsPerSecond = PHYSICS_STEPS_PER_SECOND,
    .framesPerSecondLimit = FPS_LIMIT,
    .usingImGui = true
};

WorldRoom::WorldRoom(const GameSettings& gameSettings):
    Room(1, INITIAL_SETTINGS),
    m_gameSettings(gameSettings),
    m_world(m_chunkGen),
    m_worldDrawer(engine().getWindowDims(), 32u),
    m_player(),
    m_playerInv({10, 4}),
    m_itemUser(m_world, m_playerInv),
    m_invUI(engine().getWindowDims()) {

    //InventoryUI connections
    m_invUI.connectToInventory(&m_playerInv, InventoryUI::Connection::PRIMARY);
    m_invUI.connectToItemUser(&m_itemUser);
}

void WorldRoom::sessionStart(const RE::RoomTransitionArguments& args) {
    try {
        const std::string& worldName = std::any_cast<const std::string&>(args[0]);
        if (!loadWorld(worldName)) {
            engine().scheduleRoomTransition(0, {});
            return;
        }
        engine().setWindowTitle("RealWorld! - " + worldName);
    } catch (...) {
        RE::fatalError("Bad transition paramaters to start WorldRoom session");
    }

    m_worldView.setPosition(glm::vec2(m_player.getCenter()));
}

void WorldRoom::sessionEnd() {
    //saveWorld();
}

void WorldRoom::step() {
    m_computeCommandBuffer->reset();
    m_computeCommandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    //Simulate one physics step
    performWorldSimulationStep(m_worldDrawer.setPosition(m_worldView.getBotLeft()));

    //Analyze the results of the simulation step for drawing
    analyzeWorldForDrawing();

    //Manipulate the inventory based on user's input
    updateInventoryAndUI();

    m_computeCommandBuffer->end();
    m_computeCommandBuffer.submitToComputeQueue();
}

void WorldRoom::render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) {
    m_worldDrawer.drawTiles(commandBuffer);

    m_spriteBatch.clearAndBeginFirstBatch();
    m_player.draw(m_spriteBatch);
    m_spriteBatch.drawBatch(commandBuffer, m_worldView.getViewMatrix());

    if (m_shadows) {
        m_worldDrawer.drawShadows(commandBuffer);
    }

    drawGUI(commandBuffer);
}

void WorldRoom::windowResizedCallback(const glm::ivec2& oldSize, const glm::ivec2& newSize) {
    m_worldView.resizeView(newSize);
    m_worldDrawer.resizeView(newSize);
    m_invUI.windowResized(newSize);
    m_windowViewMat = calculateWindowViewMat(newSize);
}

void WorldRoom::performWorldSimulationStep(const WorldDrawer::ViewEnvelope& viewEnvelope) {
    //Prepare for the simulation step (wait on barriers and do image layout transitions)
    m_world.beginStep(*m_computeCommandBuffer);

    //Simulate one physics step (load new chunks if required)
    m_world.step(*m_computeCommandBuffer, viewEnvelope.botLeftTi, viewEnvelope.topRightTi);

    //Modify the world with player's tools
    m_itemUser.step(
        keybindDown(ITEMUSER_USE_PRIMARY) && !m_invUI.isOpen(),
        keybindDown(ITEMUSER_USE_SECONDARY) && !m_invUI.isOpen(),
        m_worldView.getCursorRel()
    );

    //Move the player within the updated world
    m_player.step(
        *m_computeCommandBuffer,
        (keybindDown(PLAYER_LEFT) ? -1.0f : 0.0f) + (keybindDown(PLAYER_RIGHT) ? +1.0f : 0.0f),
        keybindDown(PLAYER_JUMP),
        keybindDown(PLAYER_AUTOJUMP)
    );

    //Finish the simulation step (transit image layouts back)
    m_world.endStep(*m_computeCommandBuffer);
}

void WorldRoom::analyzeWorldForDrawing() {
    //Move the view based on movements of the player
    glm::vec2 prevViewPos = m_worldView.getPosition();
    glm::vec2 targetViewPos = glm::vec2(m_player.getCenter()) * 0.75f + m_worldView.getCursorRel() * 0.25f;
    auto viewPos = prevViewPos * 0.875f + targetViewPos * 0.125f;
    m_worldView.setCursorAbs(engine().getCursorAbs());
    m_worldView.setPosition(glm::floor(viewPos));

    //Analyze the world texture
    m_worldDrawer.beginStep(*m_computeCommandBuffer);

    //Add external lights (these below are mostly for debug)
    static float rad = 0.0f;
    rad += 0.01f;
    m_worldDrawer.addExternalLight(m_worldView.getCursorRel() + glm::vec2(glm::cos(rad), glm::sin(rad)) * 0.0f, RE::Color{0u, 0u, 0u, 255u});
    m_worldDrawer.addExternalLight(m_player.getCenter(), RE::Color{0u, 0u, 0u, 100u});

    //Calculate illumination based the world texture and external lights
    m_worldDrawer.endStep(*m_computeCommandBuffer);
}

void WorldRoom::updateInventoryAndUI() {
    //Inventory
    m_invUI.step();
    if (keybindPressed(INV_OPEN_CLOSE)) { m_invUI.openOrClose(); }
    if (m_invUI.isOpen()) {//Inventory is open
        if (keybindPressed(INV_MOVE_ALL)) { m_invUI.swapUnderCursor(engine().getCursorAbs()); }
        if (keybindPressed(INV_MOVE_PORTION)) { m_invUI.movePortion(engine().getCursorAbs(), 0.5f); }
    } else { //Inventory is closed
        using enum SlotSelectionManner;
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

    //Toggles & quit
    if (keybindPressed(QUIT)) { engine().scheduleRoomTransition(0, {}); }
    if (keybindPressed(MINIMAP)) { m_minimap = !m_minimap; }
    if (keybindPressed(SHADOWS)) { m_shadows = !m_shadows; }
    if (keybindPressed(PERMUTE)) { m_world.shouldPermuteOrder(m_permute = !m_permute); }
}

void WorldRoom::drawGUI(const vk::CommandBuffer& commandBuffer) {
    //Inventory
    m_spriteBatch.nextBatch();
    m_invUI.draw(m_spriteBatch, engine().getCursorAbs());
    m_spriteBatch.drawBatch(commandBuffer, m_windowViewMat);
    //Minimap
    if (m_minimap) {
        m_worldDrawer.drawMinimap(commandBuffer);
    }
    //Top-left menu
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::PushFont(m_arial);
    if (ImGui::Begin("##topLeftMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
        ImGui::Text("FPS: %u\nMax FT: %i us",
            engine().getFramesPerSecond(),
            (int)std::chrono::duration_cast<std::chrono::microseconds>(engine().getMaxFrameTime()).count());
        ImGui::Separator();
        ImGui::TextUnformatted("Minimap:"); ImGui::SameLine();
        ImGui::ToggleButton("##minimap", &m_minimap);
        ImGui::TextUnformatted("Shadows:"); ImGui::SameLine();
        ImGui::ToggleButton("##shadows", &m_shadows);
        ImGui::TextUnformatted("Permute:"); ImGui::SameLine();
        if (ImGui::ToggleButton("##shadows", &m_permute)) m_world.shouldPermuteOrder(m_permute);
    }
    ImGui::End();
    ImGui::PopFont();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
    WorldSave save{};

    if (!WorldSaveLoader::loadWorld(save, worldName)) return false;

    const auto& worldTex = m_world.adoptSave(save.metadata, m_gameSettings.getActiveChunksArea());
    m_player.adoptSave(save.player, worldTex);
    m_playerInv.adoptInventoryData(save.inventory);

    m_worldDrawer.setTarget(worldTex, m_gameSettings.getActiveChunksArea() * iCHUNK_SIZE);
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

glm::mat4 WorldRoom::calculateWindowViewMat(const glm::vec2& windowDims) const {
    return glm::ortho(0.0f, windowDims.x, 0.0f, windowDims.y);
}
