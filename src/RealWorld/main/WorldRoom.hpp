﻿/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/graphics/cameras/View2D.hpp>
#include <RealEngine/graphics/synchronization/Semaphore.hpp>

#include <RealWorld/drawing/WorldDrawer.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/InventoryUI.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/main/Room.hpp>
#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/simulation/general/ActionCmdBuf.hpp>
#include <RealWorld/simulation/general/ShaderMessageBroker.hpp>
#include <RealWorld/simulation/objects/Player.hpp>
#include <RealWorld/simulation/tiles/World.hpp>

namespace rw {

/**
 * @brief Holds all gameplay-related objects.
 */
class WorldRoom: public Room {
public:
    explicit WorldRoom(const GameSettings& gameSettings);

    void sessionStart(const re::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(const re::CommandBuffer& cb, double interpolationFactor) override;

    void windowResizedCallback(glm::ivec2 oldSize, glm::ivec2 newSize) override;

private:
    using enum RealWorldKeyBindings;

    void performWorldSimulationStep(const WorldDrawer::ViewEnvelope& viewEnvelope);

    void analyzeWorldForDrawing();

    glm::vec2 newViewPos() const;

    void updateInventoryAndUI();

    void drawGUI(const re::CommandBuffer& cb);

    /**
     * @brief Loads a world. Previously loaded world is flushed without saving.
     *
     * @param worldName Filename of the world
     * @return True if successful, false otherwise. No change is done to current
     * world then.
     */
    bool loadWorld(const std::string& worldName);

    /**
     * @brief Saves the current world. Makes no changes to the world.
     *
     * @return True if successful, false otherwise.
     */
    bool saveWorld();

    glm::mat4 calculateWindowViewMat(glm::vec2 windowDims) const;

    const GameSettings& m_gameSettings;
    ImFont* m_arial = createFont<"arial">(20.0f); // NOLINT(*-magic-numbers)

    re::StepDoubleBuffered<re::CommandBuffer> m_stepCmdBufs{
        re::CommandBuffer{{.debugName = "rw::WorldRoom::step[0]"}},
        re::CommandBuffer{{.debugName = "rw::WorldRoom::step[1]"}}
    };
    ActionCmdBuf m_acb;
    uint64_t m_stepN = 1;
    re::Semaphore m_simulationFinishedSem{m_stepN};
    re::SpriteBatch m_spriteBatch{re::SpriteBatchCreateInfo{
        .renderPassSubpass = mainRenderPass().subpass(0),
        .maxSprites        = 256,
        .maxTextures       = 32
    }};
    re::GeometryBatch m_geometryBatch{re::GeometryBatchCreateInfo{
        .topology          = vk::PrimitiveTopology::eLineList,
        .renderPassSubpass = mainRenderPass().subpass(0),
        .maxVertices       = 1024
    }};

    // View
    re::View2D m_worldView{engine().windowDims()};
    glm::mat4 m_windowViewMat = calculateWindowViewMat(engine().windowDims());

    // Gameplay
    Inventory m_playerInv;
    ItemUser m_itemUser;
    ShaderMessageBroker m_messageBroker;
    World m_world;
    WorldDrawer m_worldDrawer;
    Player m_player;
    InventoryUI m_invUI;
    float m_timeDay    = 0.0f;
    bool m_stopDaytime = false;

    // Toggle states
    bool m_minimap = false;
    bool m_shadows = true;
};

} // namespace rw
