/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/MainMenuRoom.hpp>

#include <time.h>

#include <array>
#include <iostream>

#include <glm/common.hpp>

#include <RealWorld/save/WorldSaveLoader.hpp>

const char* KEYBIND_NOTICE = "Press a key to change the keybind.\nOr press Delete to cancel.";

glm::vec2 KEYBIND_NOTICE_SIZE() {
	return ImGui::CalcTextSize(KEYBIND_NOTICE);
}

//Lil' helper func
void controlsCategoryHeader(const char* header) {
	ImGui::TableNextRow();
	ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn();
	ImGui::TextUnformatted(header);
	ImGui::Separator(); ImGui::TableNextColumn(); ImGui::NewLine(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::NewLine(); ImGui::Separator(); ImGui::TableNextColumn();
}

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
	WorldSaveLoader::getSavedWorlds(m_worlds);
	m_newWorldName = "";
	m_newWorldSeed = static_cast<int>(time(nullptr)) & 65535;
	window()->setTitle("RealWorld!");
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

	if (ImGui::Begin("##wholeMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
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
			if (ImGui::Button("Return to main menu") || keybindReleased(QUIT)) m_menu = MAIN;
		}
	}
	ImGui::End();
	ImGui::PopFont();
}

void MainMenuRoom::keybindCallback(RE::Key newKey) {
	m_drawKeybindListeningPopup = false;
}

void MainMenuRoom::mainMenu() {
	ImGui::SetNextItemWidth(400.0f);
	if (ImGui::Button("Create a new world")) m_menu = NEW_WORLD;
	if (ImGui::Button("Load a saved world")) m_menu = LOAD_WORLD;
	if (ImGui::Button("Display settings")) m_menu = DISPLAY_SETTINGS;
	if (ImGui::Button("Controls")) m_menu = CONTROLS;

	ImGui::SetCursorPosY(window()->getDims().y - ImGui::GetFrameHeight() * 2.0f);
	ImGui::Separator();
	if (ImGui::Button("Exit") || keybindPressed(QUIT)) program()->scheduleProgramExit();
}

void MainMenuRoom::newWorldMenu() {
	ImGui::TextUnformatted("Create a new world");
	ImGui::Separator();

	ImGui::TextUnformatted("Name: "); ImGui::SameLine(); ImGui::InputText("##name", &m_newWorldName);
	ImGui::TextUnformatted("Seed: "); ImGui::SameLine(); ImGui::InputInt("##seed", &m_newWorldSeed);
	if (ImGui::Button("Create the world!")) {
		if (WorldSaveLoader::createWorld(m_newWorldName, m_newWorldSeed)) {
			program()->scheduleRoomTransition(1, {m_newWorldName});
		}
	}
}

void MainMenuRoom::loadWorldMenu() {
	ImGui::TextUnformatted("Choose a saved world to load");
	ImGui::Separator();

	ImGui::BeginTable("##worldsTable", 2);
	for (const auto& world : m_worlds) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (ImGui::Button(world.c_str())) program()->scheduleRoomTransition(1, {world});
		ImGui::TableNextColumn();
		if (ImGui::Button(("Delete##" + world).c_str())) {
			WorldSaveLoader::deleteWorld(world);
			WorldSaveLoader::getSavedWorlds(m_worlds);
		}
	}
	ImGui::EndTable();

	if (m_worlds.empty()) {
		ImGui::Text("No saved worlds...");
	}
}

void MainMenuRoom::displaySettingsMenu() {
	ImGui::TextUnformatted("Display settings");
	ImGui::Separator();

	ImGui::TextUnformatted("Fullscreen"); ImGui::SameLine();
	if (ImGui::ToggleButton("##fullscreen", &m_fullscreen)) {
		window()->goFullscreen(m_fullscreen, false);
		m_unsavedChanges = true;
	}

	if (!m_fullscreen) {
		ImGui::SameLine();
		ImGui::TextUnformatted("Borderless"); ImGui::SameLine();
		if (ImGui::ToggleButton("##borderless", &m_borderless)) {
			window()->goBorderless(m_borderless, false);
			m_unsavedChanges = true;
		}
	}

	ImGui::TextUnformatted("VSync"); ImGui::SameLine();
	if (ImGui::ToggleButton("##vSync", &m_vSync)) {
		window()->setVSync(m_vSync, false);
		m_unsavedChanges = true;
	}

	ImGui::TextUnformatted("Resolution"); ImGui::SameLine();
	ImGui::SetNextItemWidth(window()->getDims().x * 0.125f);
	if (ImGui::BeginCombo("##resolution", RESOLUTIONS_STRINGS[m_selectedResolution])) {
		for (size_t i = 0; i < RESOLUTIONS.size(); ++i) {
			if (ImGui::Selectable(RESOLUTIONS_STRINGS[i], i == m_selectedResolution)) {
				m_selectedResolution = i;
				program()->resizeWindow(RESOLUTIONS[i], false);
				m_unsavedChanges = true;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::NewLine();
	if (m_unsavedChanges && ImGui::Button("Save changes")) {
		window()->save();
		m_unsavedChanges = false;
	}
}

void MainMenuRoom::controlsMenu() {
	ImGui::TextUnformatted("Controls");
	ImGui::Separator();

	ImGui::BeginTable("##controlsTable", 3,
		ImGuiTableFlags_ScrollY, {0.0f, window()->getDims().y - ImGui::GetFrameHeight() * 4.125f});
	for (size_t i = 0; i < magic_enum::enum_count<RealWorldKeyBindings>(); i++) {
		ImGui::PushID(static_cast<int>(i));
		switch (static_cast<RealWorldKeyBindings>(i)) {
		case INV_OPEN_CLOSE: controlsCategoryHeader("Inventory"); break;
		case ITEMUSER_USE_PRIMARY: controlsCategoryHeader("Item usage"); break;
		case PLAYER_LEFT: controlsCategoryHeader("Player movement"); break;
		case QUIT: controlsCategoryHeader("Other"); break;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::TextUnformatted(KEYBINDER_DESC[i]);
		ImGui::TableNextColumn();

		RealWorldKeyBindings binding = static_cast<RealWorldKeyBindings>(i);
		if (ImGui::Button(RE::keyToString(keybinder(binding)).data())) {
			keybinder().listenChangeBinding<MainMenuRoom, &MainMenuRoom::keybindCallback>(binding, *this);
			m_drawKeybindListeningPopup = true;
		}

		if (keybinder(binding) != KEYBINDER_DEFAULT_LIST[i]) {
			ImGui::TableNextColumn();
			if (ImGui::Button("Reset")) {
				keybinder().resetBinding(static_cast<RealWorldKeyBindings>(i));
			}
		}
		ImGui::PopID();
	}

	if (m_drawKeybindListeningPopup) {
		glm::vec2 display = window()->getDims();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowContentSize(KEYBIND_NOTICE_SIZE());
		ImGui::SetNextWindowPos(display * 0.5f - KEYBIND_NOTICE_SIZE() * 0.5f);
		if (ImGui::Begin("##listening", nullptr, ImGuiWindowFlags_NoDecoration)) {
			ImGui::TextUnformatted(KEYBIND_NOTICE);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	ImGui::EndTable();
}
