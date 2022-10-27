/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/main/Room.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/rendering/basic_shaders/AllShaders.hpp>
#include <RealEngine/rendering/cameras/View2D.hpp>
#include <RealEngine/rendering/output/Viewport.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>
#include <RealWorld/player/Player.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/InventoryUI.hpp>

 //Use this to swap generators
#define FBO_GENERATOR 1
#define CS_GENERATOR 2
#define CHUNK_GENERATOR CS_GENERATOR
#if CHUNK_GENERATOR == CS_GENERATOR
#include <RealWorld/generation/ChunkGeneratorCS.hpp>
#elif CHUNK_GENERATOR == FBO_GENERATOR
#include <RealWorld/generation/ChunkGeneratorFBO.hpp>
#else
#error "No chunk generator has been selected!"
#endif

/**
 * @brief Holds all gameplay-related objects.
*/
template<RE::Renderer R>
class WorldRoom : public Room {
public:

    WorldRoom(const GameSettings& gameSettings);

    void sessionStart(const RE::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(double interpolationFactor) override;

    void windowResizedCallback(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;

private:

    using enum RealWorldKeyBindings;

    void drawGUI();

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
    bool saveWorld() const;

    glm::mat4 windowMatrix() const;

    const GameSettings& m_gameSettings;
    ImFont* m_arial = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20.0f);

    RE::SpriteBatch<R> m_sb{{.vert = RE::sprite_vert, .frag = RE::sprite_frag}};
    RE::GeometryBatch<R> m_gb{{.vert = RE::geometry_vert, .frag = RE::geometry_frag}};

    //View
    RE::View2D m_worldView{engine().getWindowDims()};
    RE::BufferTyped<R> m_worldViewBuf{UNIF_BUF_VIEWPORT_MATRIX, RE::BindNow::NO, RE::BufferUsageFlags::DYNAMIC_STORAGE, m_worldView.getViewMatrix()};
    RE::BufferTyped<R> m_guiViewBuf{UNIF_BUF_VIEWPORT_MATRIX, RE::BindNow::NO, RE::BufferUsageFlags::DYNAMIC_STORAGE, windowMatrix()};

    //Gameplay
#if CHUNK_GENERATOR == CS_GENERATOR
    ChunkGeneratorCS<R> m_chunkGen;
#elif CHUNK_GENERATOR == FBO_GENERATOR
    ChunkGeneratorFBO<R> m_chunkGen;
#endif
    World<R> m_world;
    WorldDrawer<R> m_worldDrawer;
    Player<R> m_player;
    Inventory<R> m_playerInv;
    ItemUser<R> m_itemUser;
    InventoryUI<R> m_invUI;

    //Toggle states
    bool m_minimap = false;
    bool m_shadows = true;
    bool m_permute = true;
};