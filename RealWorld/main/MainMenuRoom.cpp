/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/MainMenuRoom.hpp>

#include <time.h>

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

MainMenuRoom::MainMenuRoom(GameSettings& gameSettings) :
    Room(0, DEFAULT_SETTINGS),
    m_gameSettings(gameSettings),
    m_resolution(std::find(RESOLUTIONS.begin(), RESOLUTIONS.end(), engine().getWindowDims())),
    m_renderer(std::find(RENDERERS.begin(), RENDERERS.end(), engine().getUsedRenderer())),
    m_activeChunksArea(std::find(ACTIVE_CHUNKS_AREAS.begin(), ACTIVE_CHUNKS_AREAS.end(), m_gameSettings.getActiveChunksArea())) {

}

void MainMenuRoom::sessionStart(const RE::RoomTransitionArguments& args) {
    m_menu = MAIN;
    WorldSaveLoader::getSavedWorlds(m_worlds);
    m_newWorldName = "";
    m_newWorldSeed = static_cast<int>(time(nullptr)) & 65535;
    engine().setWindowTitle("RealWorld!");
}

void MainMenuRoom::sessionEnd() {

}

void MainMenuRoom::step() {
    //GUI only room...
}

void MainMenuRoom::render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) {
    ImGui::SetNextWindowSize(engine().getWindowDims());
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
            ImGui::SetCursorPosY(engine().getWindowDims().y - ImGui::GetFrameHeight() * 2.0f);
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

    ImGui::SetCursorPosY(engine().getWindowDims().y - ImGui::GetFrameHeight() * 2.0f);
    ImGui::Separator();
    if (ImGui::Button("Exit") || keybindPressed(QUIT)) engine().scheduleExit();
}

void MainMenuRoom::newWorldMenu() {
    ImGui::TextUnformatted("Create a new world");
    ImGui::Separator();

    ImGui::TextUnformatted("Name: "); ImGui::SameLine(); ImGui::InputText("##name", &m_newWorldName);
    ImGui::TextUnformatted("Seed: "); ImGui::SameLine(); ImGui::InputInt("##seed", &m_newWorldSeed);
    if (ImGui::Button("Create the world!")) {
        if (WorldSaveLoader::createWorld(m_newWorldName, m_newWorldSeed)) {
            engine().scheduleRoomTransition(1, {m_newWorldName});
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
        if (ImGui::Button(world.c_str())) engine().scheduleRoomTransition(1, {world});
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
        engine().setWindowFullscreen(m_fullscreen, true);
    }

    if (!m_fullscreen) {
        ImGui::SameLine(); ImGui::TextUnformatted("Borderless"); ImGui::SameLine();
        if (ImGui::ToggleButton("##borderless", &m_borderless)) {
            engine().setWindowBorderless(m_borderless, true);
        }
    }

    ImGui::SameLine(); ImGui::TextUnformatted("VSync"); ImGui::SameLine();
    if (ImGui::ToggleButton("##vSync", &m_vSync)) {
        engine().setWindowVSync(m_vSync, true);
    }

    auto width = engine().getWindowDims().x * 0.2f;
    if (comboSelect(RESOLUTIONS, "Resolution", width, m_resolution, ivec2ToString)) {
        engine().setWindowDims(*m_resolution, true);
    }

    if (comboSelect(RENDERERS, "Preferred renderer", width, m_renderer, rendererToString)) {
        engine().setPreferredRenderer(*m_renderer, true);
    }
    std::string currRenderer = "(requires restart; current: " + rendererToString(RENDERERS[static_cast<size_t>(engine().getUsedRenderer())]) + ")";
    ImGui::SameLine(); ImGui::TextUnformatted(currRenderer.c_str());

    if (comboSelect(ACTIVE_CHUNKS_AREAS, "Active chunks area", width, m_activeChunksArea, ivec2ToString)) {
        m_gameSettings.setActiveChunksArea(*m_activeChunksArea);
        m_gameSettings.save();
    }
}

void MainMenuRoom::controlsMenu() {
    ImGui::TextUnformatted("Controls");
    ImGui::Separator();

    ImGui::BeginTable("##controlsTable", 3,
        ImGuiTableFlags_ScrollY, {0.0f, engine().getWindowDims().y - ImGui::GetFrameHeight() * 4.125f});
    for (size_t i = 0; i < static_cast<size_t>(RealWorldKeyBindings::COUNT); i++) {
        ImGui::PushID(static_cast<int>(i));
        switch (static_cast<RealWorldKeyBindings>(i)) {
        case INV_OPEN_CLOSE: controlsCategoryHeader("Inventory"); break;
        case ITEMUSER_USE_PRIMARY: controlsCategoryHeader("Item usage"); break;
        case PLAYER_LEFT: controlsCategoryHeader("Player movement"); break;
        case QUIT: controlsCategoryHeader("Other"); break;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::TextUnformatted(KEYBINDING_INFO[i].desc);
        ImGui::TableNextColumn();

        RealWorldKeyBindings binding = static_cast<RealWorldKeyBindings>(i);
        if (ImGui::Button(RE::keyToString(keybinder(binding)).data())) {
            keybinder().listenChangeBinding<MainMenuRoom, &MainMenuRoom::keybindCallback>(binding, *this);
            m_drawKeybindListeningPopup = true;
        }

        if (keybinder(binding) != KEYBINDING_INFO[i].defaultValue) {
            ImGui::TableNextColumn();
            if (ImGui::Button("Reset")) {
                keybinder().resetBinding(static_cast<RealWorldKeyBindings>(i));
            }
        }
        ImGui::PopID();
    }

    if (m_drawKeybindListeningPopup) {
        glm::vec2 display = engine().getWindowDims();
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
