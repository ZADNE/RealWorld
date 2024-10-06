/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <glm/vec2.hpp>

#include <RealWorld/main/Room.hpp>
#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/main/settings/combos.hpp>

namespace rw {

/**
 * @brief Holds the main menu and related objects
 */
class MainMenuRoom: public Room {
public:
    explicit MainMenuRoom(GameSettings& gameSettings);

    void sessionStart(const re::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(const re::CommandBuffer& cb, double interpolationFactor) override;

    void keybindCallback(re::Key newKey);

private:
    glm::vec2 windowDims() const { return glm::vec2{engine().windowDims()}; }

    enum class Menu {
        Main,
        NewWorld,
        LoadWorld,
        DisplaySettings,
        Controls
    };
    using enum Menu;

    GameSettings& m_gameSettings;
    Menu m_menu = Main;                /**< The currently open menu */
    std::vector<std::string> m_worlds; /**< Names of all worlds that can be loaded */
    ImFont* m_arial16 =
        ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 28.0f);
    std::string m_newWorldName; /**< Name of the world that will be created */
    int m_newWorldSeed = 0;     /**< Seed of the world that will be created */

    bool m_fullscreen                           = engine().isWindowFullscreen();
    bool m_borderless                           = engine().isWindowBorderless();
    bool m_vSync                                = engine().isWindowVSynced();
    std::vector<std::string> m_availableDevices = engine().availableDevices();
    decltype(k_resolutions)::const_iterator m_resolution{};
    decltype(m_availableDevices)::const_iterator m_preferredDevice{};
    decltype(k_worldTexSizes)::const_iterator m_worldTexSize{};

    bool m_drawKeybindListeningPopup = false; /**< True if currently listening */

    void mainMenu();                          /**< Builds main menu */
    void newWorldMenu();        /**< Builds menu that creates new worlds */
    void loadWorldMenu();       /**< Builds menu that loads worlds */
    void displaySettingsMenu(); /**< Builds menu that changes settings */
    void controlsMenu();        /**< Builds menu that changes key bindings */
};

} // namespace rw
