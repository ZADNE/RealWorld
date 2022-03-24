#include <RealWorld/main/MainMenuRoom.hpp>

#include <array>
#include <iostream>

#include <time.h>

#include <RealEngine/main/MainProgram.hpp>

#include <RealWorld/world/WorldCreator.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>

const std::array RESOLUTIONS_STRINGS = {
	"1280x1024",
	"1360x768",
	"1366x768",
	"1440x900",
	"1600x900",
	"1680x1050",
	"1920x1080",
	"2560x1080",
	"2560x1440",
	"3440x1440",
	"3840x2160"
};

const std::array<glm::ivec2, RESOLUTIONS_STRINGS.size()> RESOLUTIONS = {
	glm::ivec2{1280, 1024},
	glm::ivec2{1360, 768},
	glm::ivec2{1366, 768},
	glm::ivec2{1440, 900},
	glm::ivec2{1600, 900},
	glm::ivec2{1680, 1050},
	glm::ivec2{1920, 1080},
	glm::ivec2{2560, 1080},
	glm::ivec2{2560, 1440},
	glm::ivec2{3440, 1440},
	glm::ivec2{3840, 2160}
};


MainMenuRoom::MainMenuRoom(RE::CommandLineArguments args) {
	glm::ivec2 windowSize = window()->getDims();
	for (size_t i = 0; i < RESOLUTIONS.size(); ++i) {
		if (windowSize == RESOLUTIONS[i]) {
			m_selectedResolution = i;
		}
	}
}

MainMenuRoom::~MainMenuRoom() {

}

void MainMenuRoom::sessionStart(const RE::RoomTransitionParameters& params) {
	m_menu = MAIN;
	WorldDataLoader::getSavedWorlds(m_worlds);
	m_newWorldName = "";
	m_newWorldSeed = static_cast<int>(time(nullptr)) & 65535;
}

void MainMenuRoom::sessionEnd() {

}

void MainMenuRoom::step() {
	//GUI only room...
}

void MainMenuRoom::render(double interpolationFactor) {
	ImGui::SetNextWindowSize(window()->getDims());
	ImGui::SetNextWindowPos({0.0f, 0.0f});
	ImGui::PushFont(m_arial16);

	if (ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoDecoration)) {
		ImGui::Indent();
		ImGui::SetCursorPosY(ImGui::GetFrameHeight());
		switch (m_menu) {
		case MAIN: mainMenu(); break;
		case NEW_WORLD: newWorldMenu(); break;
		case LOAD_WORLD: loadWorldMenu(); break;
		case DISPLAY_SETTINGS: displaySettingsMenu(); break;
		case CONTROLS: controlsMenu(); break;
		}

		if (m_menu != MAIN) {
			ImGui::SetCursorPosY(window()->getDims().y - ImGui::GetFrameHeight() * 2.0f);
			ImGui::Separator();
			if (ImGui::Button("Return to main menu")) m_menu = MAIN;
		}
	}
	ImGui::PopFont();
	ImGui::End();
}

void MainMenuRoom::mainMenu() {
	ImGui::SetNextItemWidth(400.0f);
	if (ImGui::Button("Create a new world")) m_menu = NEW_WORLD;
	if (ImGui::Button("Load a saved world")) m_menu = LOAD_WORLD;
	if (ImGui::Button("Display settings")) m_menu = DISPLAY_SETTINGS;
	if (ImGui::Button("Controls")) m_menu = CONTROLS;

	ImGui::SetCursorPosY(window()->getDims().y - ImGui::GetFrameHeight() * 2.0f);
	ImGui::Separator();
	if (ImGui::Button("Exit")) program()->scheduleProgramExit();
}

void MainMenuRoom::newWorldMenu() {
	ImGui::Text("Create a new world");
	ImGui::Separator();

	ImGui::Text("Name: "); ImGui::SameLine(); ImGui::InputText("##name", &m_newWorldName);
	ImGui::Text("Seed: "); ImGui::SameLine(); ImGui::InputInt("##seed", &m_newWorldSeed);
	if (ImGui::Button("Create the world!")) {
		auto wd = WorldCreator::createWorld(m_newWorldName, m_newWorldSeed);
		if (WorldDataLoader::saveWorldData(wd, m_newWorldName, true)) {
			program()->scheduleRoomTransition(1, {m_newWorldName});
		}
	}
}

void MainMenuRoom::loadWorldMenu() {
	ImGui::Text("Choose a saved world to load");
	ImGui::Separator();

	ImGui::BeginTable("worldsTable", 2);
	for (const auto& world : m_worlds) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (ImGui::Button(world.c_str())) program()->scheduleRoomTransition(1, {world});
		ImGui::TableNextColumn();
		if (ImGui::Button(("Delete##" + world).c_str())) {
			WorldDataLoader::deleteWorld(world);
			WorldDataLoader::getSavedWorlds(m_worlds);
		}
	}
	ImGui::EndTable();

	if (m_worlds.empty()) {
		ImGui::Text("No saved worlds...");
	}
}

void MainMenuRoom::displaySettingsMenu() {
	using namespace std::string_literals;
	ImGui::Text("Display settings");
	ImGui::Separator();

	ImGui::TextUnformatted("Fullscreen"); ImGui::SameLine();
	if (ImGui::ToggleButton("##fullscreen", &m_fullscreen)) {
		window()->goFullscreen(m_fullscreen, false);
	}

	if (!m_fullscreen){
		ImGui::SameLine();
		ImGui::TextUnformatted("Borderless"); ImGui::SameLine();
		if (ImGui::ToggleButton("##borderless", &m_borderless)) {
			window()->goBorderless(m_borderless, false);
		}
	}

	ImGui::TextUnformatted("VSync"); ImGui::SameLine();
	if (ImGui::ToggleButton("##vSync", &m_vSync)) {
		window()->setVSync(m_vSync, false);
	}
	
	if (ImGui::BeginCombo("##resolution", RESOLUTIONS_STRINGS[m_selectedResolution])) {
		for (size_t i = 0; i < RESOLUTIONS.size(); ++i) {
			if (ImGui::Selectable(RESOLUTIONS_STRINGS[i], i == m_selectedResolution)) {
				m_selectedResolution = i;
				program()->resizeWindow(RESOLUTIONS[i], false);
			}
		}
		ImGui::EndCombo();
	}

	ImGui::NewLine();
	if (ImGui::Button("Save settings")) {
		window()->save();
	}
}

void MainMenuRoom::controlsMenu() {
	ImGui::Text("Controls");
	ImGui::Separator();

}
