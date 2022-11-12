/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/main/Room.hpp>

#include <glm/vec2.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/main/settings/combos.hpp>

 /**
  * @brief Holds the main menu and related objects.
 */
class MainMenuRoom : public Room {
public:

    MainMenuRoom(GameSettings& gameSettings);

    void sessionStart(const RE::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(double interpolationFactor) override;

    void keybindCallback(RE::Key newKey);

private:

    static constexpr RE::RoomDisplaySettings DEFAULT_SETTINGS{
        .framesPerSecondLimit = 144,
        .usingImGui = true
    };

    enum class Menu {
        MAIN,
        NEW_WORLD,
        LOAD_WORLD,
        DISPLAY_SETTINGS,
        CONTROLS
    };
    using enum Menu;

    GameSettings& m_gameSettings;
    Menu m_menu = MAIN;                                 /**< The currently open menu */
    std::vector<std::string> m_worlds;                  /**< Names of all worlds that can be loaded */
    ImFont* m_arial16 = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 28.0f);
    std::string m_newWorldName;                         /**< Name of the world that will be created */
    int m_newWorldSeed = 0;                             /**< Seed of the world that will be created */

    bool m_fullscreen = engine().isWindowFullscreen();
    bool m_borderless = engine().isWindowBorderless();
    bool m_vSync = engine().isWindowVSynced();
    decltype(RESOLUTIONS)::const_iterator m_resolution;
    decltype(RENDERERS)::const_iterator m_renderer;
    decltype(ACTIVE_CHUNKS_AREAS)::const_iterator m_activeChunksArea;

    bool m_drawKeybindListeningPopup = false;           /**< True if currently listening */

    void mainMenu();                                    /**< Builds main menu */
    void newWorldMenu();                                /**< Builds menu that creates new worlds */
    void loadWorldMenu();                               /**< Builds menu that loads worlds */
    void displaySettingsMenu();                         /**< Builds menu that changes settings */
    void controlsMenu();                                /**< Builds menu that changes key bindings */
};