/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/main/WorldRoom.hpp>
#include <RealWorld/save/WorldSaveLoader.hpp>

namespace rw {

constexpr unsigned int k_frameRateLimit =
    (re::k_buildType == re::BuildType::Debug)
        ? 300u
        : re::Synchronizer::k_doNotLimitFramesPerSecond;

constexpr vk::AttachmentDescription2 k_attachmentDescription{
    {},
    re::k_surfaceFormat.format,
    vk::SampleCountFlagBits::e1,
    vk::AttachmentLoadOp::eDontCare,  // Color
    vk::AttachmentStoreOp::eStore,    // Color
    vk::AttachmentLoadOp::eDontCare,  // Stencil
    vk::AttachmentStoreOp::eDontCare, // Stencil
    vk::ImageLayout::eUndefined,      // Initial
    vk::ImageLayout::ePresentSrcKHR   // Final
};

constexpr re::RenderPassCreateInfo k_renderPassCreateInfo{
    .attachments  = {&k_attachmentDescription, 1},
    .subpasses    = {&re::default_renderpass::k_subpassDescription, 1},
    .dependencies = {&re::default_renderpass::k_subpassDependency, 1},
    .debugName    = "rw::WorldRoom::mainRenderpass"
};

WorldRoom::WorldRoom(const GameSettings& gameSettings)
    : Room(
          1,
          re::RoomDisplaySettings{
              .stepsPerSecond       = k_physicsStepsPerSecond,
              .framesPerSecondLimit = k_frameRateLimit,
              .mainRenderPass       = &k_renderPassCreateInfo,
              .imGuiSubpassIndex    = 0
          }
      )
    , m_messageBroker(m_acb, m_playerInv, m_itemUser)
    , m_world(m_messageBroker.messageBuffer())
    , m_gameSettings(gameSettings)
    , m_worldDrawer(mainRenderPass().subpass(0), engine().windowDims(), 32u)
    , m_droppedTilesDrawer(mainRenderPass().subpass(0), m_world.droppedTilesBuf())
    , m_player()
    , m_playerInv({10, 4})
    , m_itemUser(m_world, m_playerInv)
    , m_invUI(engine().windowDims()) {

    m_invUI.connectToInventory(&m_playerInv, InventoryUI::Connection::Primary);
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

    m_worldView.setPosition(m_player.centerPx());
    m_worldView.setCursorAbs(engine().cursorAbs());
    glm::vec2 viewPos = m_player.centerPx() * 0.75f + m_worldView.cursorRel() * 0.25f;
    m_worldView.setPosition(glm::floor(viewPos));
}

void WorldRoom::sessionEnd() {
    saveWorld();
}

void WorldRoom::step() {
    // Get the command buffer of the current step
    auto& cb = m_stepCmdBufs.write();
    m_acb.useCommandBuffer(cb);
    m_acb.assumeActionsFinished();

    // Wait for the command buffer to be consumed.
    // It should already be consumed thanks to RealEngine's step() timing
    m_simulationFinishedSem.wait(++m_stepN - 2);

    cb->reset();
    cb->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    {
        auto dbg = cb.createDebugRegion("step", {1.0, 0.0, 0.0, 1.0});

        // Move the view based on movements of the player
        auto viewPos = newViewPos();
        m_worldView.setCursorAbs(engine().cursorAbs());
        m_worldView.setPosition(glm::floor(viewPos));

        // Simulate one physics step
        performWorldSimulationStep(m_worldDrawer.setPosition(m_worldView.botLeft()));

        // Analyze the results of the simulation step for drawing
        analyzeWorldForDrawing();
    }
    cb->end();

    // Submit the compute work to GPU
    vk::SemaphoreSubmitInfo waitSems{
        *m_simulationFinishedSem, m_stepN - 1, vk::PipelineStageFlagBits2::eNone
    };
    vk::CommandBufferSubmitInfo comBufSubmit{*cb};
    vk::SemaphoreSubmitInfo signalSems{
        *m_simulationFinishedSem, m_stepN, vk::PipelineStageFlagBits2::eNone
    };
    re::CommandBuffer::submitToGraphicsCompQueue(
        vk::SubmitInfo2{{}, waitSems, comBufSubmit, signalSems}
    );

    // Manipulate the inventory based on user's input
    updateInventoryAndUI();
}

void WorldRoom::render(const re::CommandBuffer& cb, double interpolationFactor) {
    auto dbg = cb.createDebugRegion("render", {0.0, 0.0, 1.0, 1.0});
    engine().mainRenderPassBegin({});

    glm::mat4 mvpMat = m_worldView.viewMatrix();

    m_worldDrawer.drawTiles(cb);

    m_spriteBatch.clearAndBeginFirstBatch();
    m_player.draw(m_spriteBatch);
    m_spriteBatch.drawBatch(cb, mvpMat);

    float interpFactor = static_cast<float>(interpolationFactor);
    float stepN        = static_cast<float>(m_stepN) + interpFactor;
    m_droppedTilesDrawer.draw(
        cb, mvpMat, stepN / static_cast<float>(k_physicsStepsPerSecond), interpFactor
    );

    if (m_shadows) {
        m_worldDrawer.drawShadows(cb);
    }

    if (!m_invUI.isOpen()) {
        m_geometryBatch.begin();
        m_itemUser.render(m_worldView.cursorRel(), m_geometryBatch);
        m_geometryBatch.end();
        m_geometryBatch.draw(cb, mvpMat);
    }

    drawGUI(cb);

    engine().mainRenderPassEnd();
}

void WorldRoom::windowResizedCallback(glm::ivec2 oldSize, glm::ivec2 newSize) {
    m_worldView.resizeView(newSize);
    m_worldDrawer.resizeView(newSize);
    m_invUI.windowResized(newSize);
    m_windowViewMat = calculateWindowViewMat(newSize);
}

void WorldRoom::performWorldSimulationStep(const WorldDrawer::ViewEnvelope& viewEnvelope
) {
    auto dbg = m_acb->createDebugRegion("simulation");

    // Process messages from 2-previous step
    m_messageBroker.beginStep(m_acb);

    // Simulate one physics step (load new chunks if required)
    m_world.step(m_acb, viewEnvelope.botLeftTi, viewEnvelope.topRightTi);

    // Modify the world with player's tools
    m_itemUser.step(
        m_acb, m_invUI.selectedSlot(),
        keybindDown(ItemuserUsePrimary) && !m_invUI.isOpen(),
        keybindDown(ItemuserUseSecondary) && !m_invUI.isOpen(),
        m_worldView.cursorRel()
    );

    // Move the player within the updated world
    m_player.step(
        m_acb,
        (keybindDown(PlayerLeft) ? -1.0f : 0.0f) +
            (keybindDown(PlayerRight) ? +1.0f : 0.0f),
        keybindDown(PlayerJump), keybindDown(PlayerAutojump)
    );

    // Send messages collected this step
    m_messageBroker.endStep(m_acb);

    // Finish the simulation step (transit image layouts back)
    m_world.prepareWorldForDrawing(m_acb);
}

void WorldRoom::analyzeWorldForDrawing() {
    auto dbg = m_acb->createDebugRegion("analysisForDrawing");

    // Analyze the world texture
    m_timeDay += 0.00025f * static_cast<float>(!m_stopDaytime);
    m_worldDrawer.beginStep(*m_acb, m_timeDay);

    // Add external lights (these below are mostly for debug)
    m_worldDrawer.addExternalLight(
        m_worldView.cursorRel(), re::Color{0u, 0u, 0u, 255u}
    );
    m_worldDrawer.addExternalLight(m_player.centerPx(), re::Color{0u, 0u, 0u, 100u});

    // Calculate illumination based the world texture and external lights
    m_worldDrawer.endStep(*m_acb);
}

void WorldRoom::updateInventoryAndUI() {
    // Inventory
    m_invUI.step();
    if (keybindPressed(InvOpenClose)) {
        m_invUI.switchOpenClose();
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
}

void WorldRoom::drawGUI(const re::CommandBuffer& cb) {
    // Inventory
    m_spriteBatch.nextBatch();
    m_invUI.draw(m_spriteBatch, engine().cursorAbs());
    m_spriteBatch.drawBatch(cb, m_windowViewMat);
    //  Minimap
    if (m_minimap) {
        m_worldDrawer.drawMinimap(cb);
    }
    // Top-left menu
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::PushFont(m_arial);
    if (ImGui::Begin(
            "##topLeftMenu", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        )) {
        ImGui::Text(
            "FPS: %u\nMax FT: %i us", engine().framesPerSecond(),
            static_cast<int>(
                duration_cast<std::chrono::microseconds>(engine().maxFrameTime())
                    .count()
            )
        );
        ImGui::Separator();
        ImGui::TextUnformatted("Minimap:");
        ImGui::SameLine();
        ImGui::ToggleButton("##minimap", &m_minimap);
        ImGui::TextUnformatted("Shadows:");
        ImGui::SameLine();
        ImGui::ToggleButton("##shadows", &m_shadows);
        ImGui::TextUnformatted("Stop daytime:");
        ImGui::SameLine();
        ImGui::ToggleButton("##stopDaytime", &m_stopDaytime);
    }
    ImGui::End();
    ImGui::PopFont();
    engine().mainRenderPassDrawImGui();
}

bool WorldRoom::loadWorld(const std::string& worldName) {
    WorldSave save{};

    if (!WorldSaveLoader::loadWorld(save, worldName))
        return false;

    const auto& worldTex =
        m_world.adoptSave(m_acb, save.metadata, m_gameSettings.worldTexSize());
    m_player.adoptSave(save.player, worldTex, m_gameSettings.worldTexSize());
    m_playerInv.adoptInventoryData(save.inventory);
    m_timeDay = save.metadata.timeDay;

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
    return m_world.saveChunks(m_acb);
}

glm::mat4 WorldRoom::calculateWindowViewMat(glm::vec2 windowDims) const {
    return glm::ortho(0.0f, windowDims.x, 0.0f, windowDims.y);
}

} // namespace rw
