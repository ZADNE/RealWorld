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

	virtual const DisplaySettings& getDisplaySettings() override {
		static DisplaySettings settings{
			.framesPerSecondLimit = 144,
			.usingImGui = true
		};
		return settings;
	}

private:
	enum class Menu {
		MAIN,
		NEW_WORLD,
		LOAD_WORLD,
		DISPLAY_SETTINGS,
		CONTROLS
	};
	using enum Menu;

	Menu m_menu = MAIN;
	std::vector<std::string> m_worlds;
	ImFont* m_arial16 = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 28.0f);
	std::string m_newWorldName;
	int m_newWorldSeed = 0;

	bool m_unsavedChanges = false;
	bool m_fullscreen = window()->isFullscreen();
	bool m_borderless = window()->isBorderless();
	bool m_vSync = window()->isVSynced();
	size_t m_selectedResolution = 0;

	void mainMenu();
	void newWorldMenu();
	void loadWorldMenu();
	void displaySettingsMenu();
	void controlsMenu();
};