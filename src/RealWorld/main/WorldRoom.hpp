/*!
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
#include <RealWorld/player/Player.hpp>
#include <RealWorld/world/World.hpp>

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
    void render(const vk::CommandBuffer& cmdBuf, double interpolationFactor) override;

    void windowResizedCallback(
        const glm::ivec2& oldSize, const glm::ivec2& newSize
    ) override;

private:
    using enum RealWorldKeyBindings;

    void performWorldSimulationStep(
        const vk::CommandBuffer& cmdBuf, const WorldDrawer::ViewEnvelope& viewEnvelope
    );

    void analyzeWorldForDrawing(const vk::CommandBuffer& cmdBuf);

    void updateInventoryAndUI();

    void drawGUI(const vk::CommandBuffer& cmdBuf);

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

    glm::mat4 calculateWindowViewMat(const glm::vec2& windowDims) const;

    const GameSettings& m_gameSettings;
    ImFont*             m_arial =
        ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20.0f);

    re::StepDoubleBuffered<re::CommandBuffer> m_stepCommandBuffer{
        re::CommandBuffer{vk::CommandBufferLevel::ePrimary},
        re::CommandBuffer{vk::CommandBufferLevel::ePrimary}};
    uint64_t        m_stepN = 1;
    re::Semaphore   m_simulationFinishedSem{m_stepN};
    re::SpriteBatch m_spriteBatch{256, 64};
    re::GeometryBatch m_geometryBatch{vk::PrimitiveTopology::eLineList, 1024u, 1.0f};

    // View
    re::View2D m_worldView{engine().windowDims()};
    glm::mat4  m_windowViewMat = calculateWindowViewMat(engine().windowDims());

    // Gameplay
    World       m_world;
    WorldDrawer m_worldDrawer;
    Player      m_player;
    Inventory   m_playerInv;
    ItemUser    m_itemUser;
    InventoryUI m_invUI;

    // Toggle states
    bool m_minimap = false;
    bool m_shadows = true;
    bool m_permute = true;
};

} // namespace rw