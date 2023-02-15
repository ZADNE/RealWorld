/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/main/Room.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/rendering/cameras/View2D.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/generation/ChunkGeneratorCS.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>
#include <RealWorld/player/Player.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/InventoryUI.hpp>

 /**
  * @brief Holds all gameplay-related objects.
 */
class WorldRoom: public Room {
public:

    WorldRoom(const GameSettings& gameSettings);

    void sessionStart(const RE::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) override;

    void windowResizedCallback(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;

private:

    using enum RealWorldKeyBindings;

    void performWorldSimulationStep(const WorldDrawer::ViewEnvelope& viewEnvelope);

    void analyzeWorldForDrawing();

    void updateInventoryAndUI();


    void drawGUI(const vk::CommandBuffer& commandBuffer);

    /**
     * @brief Loads a world. Previously loaded world is flushed without saving.
     *
     * @param worldName Filename of the world
     * @return True if successful, false otherwise. No change is done to current world then.
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
    ImFont* m_arial = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20.0f);

    RE::CommandBuffer m_computeCommandBuffer{vk::CommandBufferLevel::ePrimary};
    RE::SpriteBatch m_spriteBatch{256, 64};
    RE::GeometryBatch m_geometryBatch{vk::PrimitiveTopology::eLineList, 1024};

    //View
    RE::View2D m_worldView{engine().getWindowDims()};
    glm::mat4 m_windowViewMat = calculateWindowViewMat(engine().getWindowDims());

    //Gameplay
    ChunkGeneratorCS m_chunkGen;
    World m_world;
    WorldDrawer m_worldDrawer;
    Player m_player;
    Inventory m_playerInv;
    ItemUser m_itemUser;
    InventoryUI m_invUI;

    //Toggle states
    bool m_minimap = false;
    bool m_shadows = true;
    bool m_permute = true;
};