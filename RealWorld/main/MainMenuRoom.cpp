/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/MainMenuRoom.hpp>

#include <ctime>

#include <RealWorld/save/WorldSaveLoader.hpp>

const char* k_keybindNotice = "Press a key to change the keybind.\nOr press Delete to cancel.";

glm::vec2 keybindNoticeSize() {
    return ImGui::CalcTextSize(k_keybindNotice);
}

//Lil' helper func
void controlsCategoryHeader(const char* header) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::Separator(); ImGui::TableNextColumn();
    ImGui::TextUnformatted(header);
    ImGui::Separator(); ImGui::TableNextColumn(); ImGui::NewLine(); ImGui::Separator(); ImGui::TableNextColumn(); ImGui::NewLine(); ImGui::Separator(); ImGui::TableNextColumn();
}

static constexpr re::RoomDisplaySettings k_initialSettings{
    .framesPerSecondLimit = 144,
        .usingImGui = true
};

MainMenuRoom::MainMenuRoom(GameSettings& gameSettings) :
    Room(0, k_initialSettings),
    m_gameSettings(gameSettings),
    m_resolution(std::find(k_resolutions.begin(), k_resolutions.end(), engine().windowDims())),
    m_activeChunksArea(std::find(k_activeChunkAreas.begin(), k_activeChunkAreas.end(), m_gameSettings.activeChunksArea())) {

}

void MainMenuRoom::sessionStart(const re::RoomTransitionArguments& args) {
    m_menu = Main;
    WorldSaveLoader::searchSavedWorlds(m_worlds);
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
    ImGui::SetNextWindowSize(engine().windowDims());
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::PushFont(m_arial16);

    if (ImGui::Begin("##wholeMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
        ImGui::Indent();
        ImGui::SetCursorPosY(ImGui::GetFrameHeight());
        switch (m_menu) {
        case Main: mainMenu(); break;
        case NewWorld: newWorldMenu(); break;
        case LoadWorld: loadWorldMenu(); break;
        case DisplaySettings: displaySettingsMenu(); break;
        case Controls: controlsMenu(); break;
        }

        if (m_menu != Main) {
            ImGui::SetCursorPosY(engine().windowDims().y - ImGui::GetFrameHeight() * 2.0f);
            ImGui::Separator();
            if (ImGui::Button("Return to main menu") || keybindReleased(Quit)) m_menu = Main;
        }
    }
    ImGui::End();
    ImGui::PopFont();
}

void MainMenuRoom::keybindCallback(re::Key newKey) {
    m_drawKeybindListeningPopup = false;
}

void MainMenuRoom::mainMenu() {
    ImGui::SetNextItemWidth(400.0f);
    if (ImGui::Button("Create a new world")) m_menu = NewWorld;
    if (ImGui::Button("Load a saved world")) m_menu = LoadWorld;
    if (ImGui::Button("Display settings")) m_menu = DisplaySettings;
    if (ImGui::Button("Controls")) m_menu = Controls;

    ImGui::SetCursorPosY(engine().windowDims().y - ImGui::GetFrameHeight() * 2.0f);
    ImGui::Separator();
    if (ImGui::Button("Exit") || keybindPressed(Quit)) engine().scheduleExit();
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
            WorldSaveLoader::searchSavedWorlds(m_worlds);
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

    auto width = engine().windowDims().x * 0.2f;
    if (comboSelect(k_resolutions, "Resolution", width, m_resolution, ivec2ToString)) {
        engine().setWindowDims(*m_resolution, true);
    }

    if (comboSelect(k_activeChunkAreas, "Active chunks area", width, m_activeChunksArea, ivec2ToString)) {
        m_gameSettings.setActiveChunksArea(*m_activeChunksArea);
        m_gameSettings.save();
    }
}

void MainMenuRoom::controlsMenu() {
    ImGui::TextUnformatted("Controls");
    ImGui::Separator();

    ImGui::BeginTable("##controlsTable", 3,
        ImGuiTableFlags_ScrollY, {0.0f, engine().windowDims().y - ImGui::GetFrameHeight() * 4.125f});
    for (size_t i = 0; i < static_cast<size_t>(RealWorldKeyBindings::Count); i++) {
        ImGui::PushID(static_cast<int>(i));
        switch (static_cast<RealWorldKeyBindings>(i)) {
        case InvOpenClose: controlsCategoryHeader("Inventory"); break;
        case ItemuserUsePrimary: controlsCategoryHeader("Item usage"); break;
        case PlayerLeft: controlsCategoryHeader("Player movement"); break;
        case Quit: controlsCategoryHeader("Other"); break;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::TextUnformatted(k_keybindingInfo[i].desc);
        ImGui::TableNextColumn();

        RealWorldKeyBindings binding = static_cast<RealWorldKeyBindings>(i);
        if (ImGui::Button(re::keyToString(keybinder(binding)).data())) {
            keybinder().listenChangeBinding<MainMenuRoom, &MainMenuRoom::keybindCallback>(binding, *this);
            m_drawKeybindListeningPopup = true;
        }

        if (keybinder(binding) != k_keybindingInfo[i].defaultValue) {
            ImGui::TableNextColumn();
            if (ImGui::Button("Reset")) {
                keybinder().resetBinding(static_cast<RealWorldKeyBindings>(i));
            }
        }
        ImGui::PopID();
    }

    if (m_drawKeybindListeningPopup) {
        glm::vec2 display = engine().windowDims();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowContentSize(keybindNoticeSize());
        ImGui::SetNextWindowPos(display * 0.5f - keybindNoticeSize() * 0.5f);
        if (ImGui::Begin("##listening", nullptr, ImGuiWindowFlags_NoDecoration)) {
            ImGui::TextUnformatted(k_keybindNotice);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    ImGui::EndTable();
}
