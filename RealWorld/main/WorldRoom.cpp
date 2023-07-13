/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/main/WorldRoom.hpp>
#include <RealWorld/save/WorldSaveLoader.hpp>

namespace rw {

#ifdef _DEBUG
constexpr unsigned int k_frameRateLimit = 300u;
#else
constexpr unsigned int k_frameRateLimit = re::Synchronizer::k_doNotLimitFramesPerSecond;
#endif // _DEBUG

constexpr glm::vec4 k_skyBlue =
    glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

constexpr re::RoomDisplaySettings k_initialSettings{
    .clearColor           = k_skyBlue,
    .stepsPerSecond       = k_physicsStepsPerSecond,
    .framesPerSecondLimit = k_frameRateLimit,
    .usingImGui           = true};

WorldRoom::WorldRoom(const GameSettings& gameSettings)
    : Room(1, k_initialSettings)
    , m_gameSettings(gameSettings)
    , m_worldDrawer(engine().windowDims(), 32u)
    , m_player()
    , m_playerInv({10, 4})
    , m_itemUser(m_world, m_playerInv)
    , m_invUI(engine().windowDims()) {

    // InventoryUI connections
    m_invUI.connectToInventory(&m_playerInv, InventoryUI::Connection::Primary);
    m_invUI.connectToItemUser(&m_itemUser);
}

void WorldRoom::sessionStart(const re::RoomTransitionArguments& args) {
    try {
        const std::string& worldName = std::any_cast<const std::string&>(args[0]);
        if (!loadWorld(worldName)) {
            engine().scheduleRoomTransition(0, {});
            return;
        }
        engine().setWindowTitle("RealWorld! - " + worldName);
    } catch (...) {
        re::fatalError("Bad transition paramaters to start WorldRoom session");
    }

    m_worldView.setPosition(m_player.center());
    m_worldView.setCursorAbs(engine().cursorAbs());
    glm::vec2 viewPos = m_player.center() * 0.75f + m_worldView.cursorRel() * 0.25f;
    m_worldView.setPosition(glm::floor(viewPos));
}

void WorldRoom::sessionEnd() {
    saveWorld();
}

void WorldRoom::step() {
    // Get the command buffer of the current step
    auto& commandBuffer = m_computeCommandBuffer[(++m_stepN) & 1];

    // Wait for the command buffer to be consumed.
    // It should already be consumed thanks to RealEngine's step() timing
    m_simulationFinishedSem.wait(m_stepN - 2);

    commandBuffer->reset();
    commandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    // Simulate one physics step
    performWorldSimulationStep(
        *commandBuffer, m_worldDrawer.setPosition(m_worldView.botLeft())
    );

    // Analyze the results of the simulation step for drawing
    analyzeWorldForDrawing(*commandBuffer);

    // Submit the compute work to GPU
    commandBuffer->end();
    vk::SemaphoreSubmitInfo waitSems{
        *m_simulationFinishedSem,
        m_stepN - 1,
        vk::PipelineStageFlagBits2::eComputeShader};
    vk::CommandBufferSubmitInfo comBufSubmit{*commandBuffer};
    vk::SemaphoreSubmitInfo     signalSems{
        *m_simulationFinishedSem,
        m_stepN,
        vk::PipelineStageFlagBits2::eComputeShader};
    re::CommandBuffer::submitToComputeQueue(vk::SubmitInfo2{
        {}, waitSems, comBufSubmit, signalSems});

    // Manipulate the inventory based on user's input
    updateInventoryAndUI();

    m_treesPoC.step();
}

void WorldRoom::render(
    const vk::CommandBuffer& commandBuffer, double interpolationFactor
) {
    m_worldDrawer.drawTiles(commandBuffer);

    m_spriteBatch.clearAndBeginFirstBatch();
    m_player.draw(m_spriteBatch);
    m_spriteBatch.drawBatch(commandBuffer, m_worldView.viewMatrix());

    if (m_shadows) {
        m_worldDrawer.drawShadows(commandBuffer);
    }

    m_geometryBatch.begin();
    m_itemUser.render(m_worldView.cursorRel(), m_geometryBatch);
    m_geometryBatch.end();
    m_geometryBatch.draw(commandBuffer, m_worldView.viewMatrix());

    drawGUI(commandBuffer);
}

void WorldRoom::windowResizedCallback(
    const glm::ivec2& oldSize, const glm::ivec2& newSize
) {
    m_worldView.resizeView(newSize);
    m_worldDrawer.resizeView(newSize);
    m_invUI.windowResized(newSize);
    m_windowViewMat = calculateWindowViewMat(newSize);
}

void WorldRoom::performWorldSimulationStep(
    const vk::CommandBuffer& commandBuffer, const WorldDrawer::ViewEnvelope& viewEnvelope
) {
    // Prepare for the simulation step
    m_world.beginStep(commandBuffer);

    // Simulate one physics step (load new chunks if required)
    m_world.step(commandBuffer, viewEnvelope.botLeftTi, viewEnvelope.topRightTi);

    // Modify the world with player's tools
    m_itemUser.step(
        commandBuffer,
        keybindDown(ItemuserUsePrimary) && !m_invUI.isOpen(),
        keybindDown(ItemuserUseSecondary) && !m_invUI.isOpen(),
        m_worldView.cursorRel()
    );

    // Move the player within the updated world
    m_player.step(
        commandBuffer,
        (keybindDown(PlayerLeft) ? -1.0f : 0.0f) +
            (keybindDown(PlayerRight) ? +1.0f : 0.0f),
        keybindDown(PlayerJump),
        keybindDown(PlayerAutojump)
    );

    // Finish the simulation step (transit image layouts back)
    m_world.endStep(commandBuffer);
}

void WorldRoom::analyzeWorldForDrawing(const vk::CommandBuffer& commandBuffer) {
    // Move the view based on movements of the player
    glm::vec2 prevViewPos   = m_worldView.center();
    glm::vec2 targetViewPos = glm::vec2(m_player.center()) * 0.75f +
                              m_worldView.cursorRel() * 0.25f;
    auto viewPos = prevViewPos * 0.875f + targetViewPos * 0.125f;
    m_worldView.setCursorAbs(engine().cursorAbs());
    m_worldView.setPosition(glm::floor(viewPos));

    // Analyze the world texture
    m_worldDrawer.beginStep(commandBuffer);

    // Add external lights (these below are mostly for debug)
    static float rad = 0.0f;
    rad += 0.01f;
    m_worldDrawer.addExternalLight(
        m_worldView.cursorRel() + glm::vec2(glm::cos(rad), glm::sin(rad)) * 0.0f,
        re::Color{0u, 0u, 0u, 255u}
    );
    m_worldDrawer.addExternalLight(m_player.center(), re::Color{0u, 0u, 0u, 100u});

    // Calculate illumination based the world texture and external lights
    m_worldDrawer.endStep(commandBuffer);
}

void WorldRoom::updateInventoryAndUI() {
    // Inventory
    m_invUI.step();
    if (keybindPressed(InvOpenClose)) {
        m_invUI.openOrClose();
    }
    if (m_invUI.isOpen()) { // Inventory is open
        if (keybindPressed(InvMoveAll)) {
            m_invUI.swapUnderCursor(engine().cursorAbs());
        }
        if (keybindPressed(InvMovePortion)) {
            m_invUI.movePortion(engine().cursorAbs(), 0.5f);
        }
    } else { // Inventory is closed
        using enum InventoryUI::SlotSelectionManner;
        if (keybindDown(ItemuserHoldToResize)) {
            if (keybindPressed(ItemuserWiden)) {
                m_itemUser.resizeShape(1.0f);
            }
            if (keybindPressed(ItemuserShrink)) {
                m_itemUser.resizeShape(-1.0f);
            }
        } else {
            if (keybindPressed(InvRightSlot)) {
                m_invUI.selectSlot(ScrollRight, keybindPressed(InvRightSlot));
            }
            if (keybindPressed(InvLeftSlot)) {
                m_invUI.selectSlot(ScrollLeft, keybindPressed(InvLeftSlot));
            }
        }
        if (keybindPressed(InvPrevSlot)) {
            m_invUI.selectSlot(ToPrevious, 0);
        }

        int slot0 = static_cast<int>(InvSlot0);
        for (int i = 0; i < 10; ++i) {
            if (keybindPressed(static_cast<RealWorldKeyBindings>(slot0 + i))) {
                m_invUI.selectSlot(AbsolutePos, i);
            }
        }
        if (keybindPressed(ItemuserSwitchShape)) {
            m_itemUser.switchShape();
        }
    }

    // Toggles & quit
    if (keybindPressed(Quit)) {
        engine().scheduleRoomTransition(0, {});
    }
    if (keybindPressed(Minimap)) {
        m_minimap = !m_minimap;
    }
    if (keybindPressed(Shadows)) {
        m_shadows = !m_shadows;
    }
    if (keybindPressed(Permute)) {
        m_world.shouldPermuteOrder(m_permute = !m_permute);
    }
}

void WorldRoom::drawGUI(const vk::CommandBuffer& commandBuffer) {
    // Inventory
    m_spriteBatch.nextBatch();
    m_invUI.draw(m_spriteBatch, engine().cursorAbs());
    m_spriteBatch.drawBatch(commandBuffer, m_windowViewMat);
    m_treesPoC.draw(commandBuffer, m_windowViewMat);
    // Minimap
    if (m_minimap) {
        m_worldDrawer.drawMinimap(commandBuffer);
    }
    // Top-left menu
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::PushFont(m_arial);
    if (ImGui::Begin("##topLeftMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
        using namespace std::chrono;
        ImGui::Text(
            "FPS: %u\nMax FT: %i us",
            engine().framesPerSecond(),
            (int)duration_cast<microseconds>(engine().maxFrameTime()).count()
        );
        ImGui::Separator();
        ImGui::TextUnformatted("Minimap:");
        ImGui::SameLine();
        ImGui::ToggleButton("##minimap", &m_minimap);
        ImGui::TextUnformatted("Shadows:");
        ImGui::SameLine();
        ImGui::ToggleButton("##shadows", &m_shadows);
        ImGui::TextUnformatted("Permute:");
        ImGui::SameLine();
        if (ImGui::ToggleButton("##permute", &m_permute))
            m_world.shouldPermuteOrder(m_permute);
    }
    ImGui::End();
    ImGui::PopFont();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
    WorldSave save{};

    if (!WorldSaveLoader::loadWorld(save, worldName))
        return false;

    const auto& worldTex =
        m_world.adoptSave(save.metadata, m_gameSettings.worldTexSize());
    m_player.adoptSave(save.player, worldTex);
    m_playerInv.adoptInventoryData(save.inventory);

    m_worldDrawer.setTarget(worldTex, m_gameSettings.worldTexSize() * iChunkTi);
    return true;
}

bool WorldRoom::saveWorld() {
    WorldSave save{};
    m_world.gatherSave(save.metadata);
    m_player.gatherSave(save.player);
    m_playerInv.gatherInventoryData(save.inventory);
    if (!WorldSaveLoader::saveWorld(save, save.metadata.worldName, false))
        return false;
    return m_world.saveChunks();
}

glm::mat4 WorldRoom::calculateWindowViewMat(const glm::vec2& windowDims) const {
    return glm::ortho(0.0f, windowDims.x, 0.0f, windowDims.y);
}

} // namespace rw