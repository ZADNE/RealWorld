#pragma once
#include <RealWorld/main/Room.hpp>

#include <glm/vec2.hpp>

#include <RealEngine/external/ImGui/imgui.h>
#include <RealEngine/external/ImGui/imgui_stdlib.h>

/**
 * @brief The room that holds the main menu and some related objects.
*/
class MainMenuRoom : public Room {
public:
	MainMenuRoom(RE::CommandLineArguments args);
	~MainMenuRoom();

	virtual void sessionStart(const RE::RoomTransitionParameters& params) override;
	virtual void sessionEnd() override;
	virtual void step() override;
	virtual void render(double interpolationFactor) override;

	virtual const RE::RoomDisplaySettings& getDisplaySettings() override {
		static RE::RoomDisplaySettings settings{
			.framesPerSecondLimit = 144,
			.usingImGui = true
		};
		return settings;
	}

	void keybindCallback(RE::Key newKey);

private:
	enum class Menu {
		MAIN,
		NEW_WORLD,
		LOAD_WORLD,
		DISPLAY_SETTINGS,
		CONTROLS
	};
	using enum Menu;

	Menu m_menu = MAIN;								/**< The currently open menu */
	std::vector<std::string> m_worlds;				/**< Names of all worlds that can be loaded */
	ImFont* m_arial16 = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 28.0f);
	std::string m_newWorldName;						/**< Name of the world that will be created */
	int m_newWorldSeed = 0;							/**< Seed of the world that will be created */

	bool m_unsavedChanges = false;					/**< True if anything has been changed in settings */
	bool m_fullscreen = window()->isFullscreen();	/**< Current state */
	bool m_borderless = window()->isBorderless();	/**< Current state */
	bool m_vSync = window()->isVSynced();			/**< Current state */
	size_t m_selectedResolution = 0;				/**< Index into the array of supported resolutions */

	bool m_drawKeybindListeningPopup = false;		/**< True if currently listening */

	void mainMenu();								/**< Builds main menu */
	void newWorldMenu();							/**< Builds menu that creates new worlds */
	void loadWorldMenu();							/**< Builds menu that loads worlds */
	void displaySettingsMenu();						/**< Builds menu that changes settings */
	void controlsMenu();							/**< Builds menu that changes key bindings */
};