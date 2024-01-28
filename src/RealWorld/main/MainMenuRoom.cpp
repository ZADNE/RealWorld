/*!
 *  @author    Dubsky Tomas
 */
#include <ctime>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/save/WorldSaveLoader.hpp>

using namespace ImGui;

namespace rw {

const char* k_keybindNotice =
    "Press a key to change the keybind.\nOr press Delete to cancel.";

glm::vec2 keybindNoticeSize() {
    return CalcTextSize(k_keybindNotice);
}

// Lil' helper func
void controlsCategoryHeader(const char* header) {
    TableNextRow();
    TableNextColumn();
    Separator();
    TableNextColumn();
    Separator();
    TableNextColumn();
    Separator();
    TableNextColumn();
    TextUnformatted(header);
    Separator();
    TableNextColumn();
    NewLine();
    Separator();
    TableNextColumn();
    NewLine();
    Separator();
    TableNextColumn();
}

static constexpr re::RoomDisplaySettings k_initialSettings{
    .framesPerSecondLimit = 144, .usingImGui = true};

MainMenuRoom::MainMenuRoom(GameSettings& gameSettings)
    : Room(0, k_initialSettings)
    , m_gameSettings(gameSettings)
    , m_resolution(std::find(
          k_resolutions.begin(), k_resolutions.end(), engine().windowDims()
      ))
    , m_worldTexSize(std::find(
          k_worldTexSizes.begin(), k_worldTexSizes.end(), m_gameSettings.worldTexSize()
      )) {
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
    // GUI only room...
}

void MainMenuRoom::render(const re::CommandBuffer& cmdBuf, double interpolationFactor) {
    engine().mainRenderPassBegin();

    SetNextWindowSize(engine().windowDims());
    SetNextWindowPos({0.0f, 0.0f});
    PushFont(m_arial16);

    if (Begin("##wholeMenu", nullptr, ImGuiWindowFlags_NoDecoration)) {
        Indent();
        SetCursorPosY(GetFrameHeight());
        switch (m_menu) {
        case Main: mainMenu(); break;
        case NewWorld: newWorldMenu(); break;
        case LoadWorld: loadWorldMenu(); break;
        case DisplaySettings: displaySettingsMenu(); break;
        case Controls: controlsMenu(); break;
        }

        if (m_menu != Main) {
            SetCursorPosY(engine().windowDims().y - GetFrameHeight() * 2.0f);
            Separator();
            if (Button("Return to main menu") || keybindReleased(Quit))
                m_menu = Main;
        }
    }
    End();
    PopFont();

    engine().mainRenderPassDrawImGui();
    engine().mainRenderPassEnd();
}

void MainMenuRoom::keybindCallback(re::Key newKey) {
    m_drawKeybindListeningPopup = false;
}

void MainMenuRoom::mainMenu() {
    SetNextItemWidth(400.0f);
    if (Button("Create a new world"))
        m_menu = NewWorld;
    if (Button("Load a saved world"))
        m_menu = LoadWorld;
    if (Button("Display settings"))
        m_menu = DisplaySettings;
    if (Button("Controls"))
        m_menu = Controls;

    SetCursorPosY(engine().windowDims().y - GetFrameHeight() * 2.0f);
    Separator();
    if (Button("Exit") || keybindPressed(Quit))
        engine().scheduleExit();
}

void MainMenuRoom::newWorldMenu() {
    TextUnformatted("Create a new world");
    Separator();

    TextUnformatted("Name: ");
    SameLine();
    InputText("##name", &m_newWorldName);
    TextUnformatted("Seed: ");
    SameLine();
    InputInt("##seed", &m_newWorldSeed);
    if (Button("Create the world!") || engine().wasKeyPressed(re::Key::Return)) {
        if (WorldSaveLoader::createWorld(m_newWorldName, m_newWorldSeed)) {
            engine().scheduleRoomTransition(1, {m_newWorldName});
        }
    }
}

void MainMenuRoom::loadWorldMenu() {
    TextUnformatted("Choose a saved world to load");
    Separator();

    BeginTable("##worldsTable", 2);
    for (const auto& world : m_worlds) {
        TableNextRow();
        TableNextColumn();
        if (Button(world.c_str()))
            engine().scheduleRoomTransition(1, {world});
        TableNextColumn();
        if (Button(("Delete##" + world).c_str())) {
            WorldSaveLoader::deleteWorld(world);
            WorldSaveLoader::searchSavedWorlds(m_worlds);
        }
    }
    EndTable();

    if (m_worlds.empty()) {
        Text("No saved worlds...");
    }
}

void MainMenuRoom::displaySettingsMenu() {
    TextUnformatted("Display settings");
    Separator();

    TextUnformatted("Fullscreen");
    SameLine();
    if (ToggleButton("##fullscreen", &m_fullscreen)) {
        engine().setWindowFullscreen(m_fullscreen, true);
    }

    if (!m_fullscreen) {
        SameLine();
        TextUnformatted("Borderless");
        SameLine();
        if (ToggleButton("##borderless", &m_borderless)) {
            engine().setWindowBorderless(m_borderless, true);
        }
    }

    SameLine();
    TextUnformatted("VSync");
    SameLine();
    if (ToggleButton("##vSync", &m_vSync)) {
        engine().setWindowVSync(m_vSync, true);
    }

    auto width = engine().windowDims().x * 0.2f;
    if (comboSelect(k_resolutions, "Resolution", width, m_resolution, ivec2ToString)) {
        engine().setWindowDims(*m_resolution, true);
    }

    if (comboSelect(
            k_worldTexSizes, "World texture size", width, m_worldTexSize, ivec2ToString
        )) {
        m_gameSettings.setWorldTexSize(*m_worldTexSize);
        m_gameSettings.save();
    }
}

void MainMenuRoom::controlsMenu() {
    TextUnformatted("Controls");
    Separator();

    BeginTable(
        "##controlsTable",
        3,
        ImGuiTableFlags_ScrollY,
        {0.0f, engine().windowDims().y - GetFrameHeight() * 4.125f}
    );
    for (size_t i = 0; i < static_cast<size_t>(RealWorldKeyBindings::Count); i++) {
        PushID(static_cast<int>(i));
        switch (static_cast<RealWorldKeyBindings>(i)) {
        case InvOpenClose: controlsCategoryHeader("Inventory"); break;
        case ItemuserUsePrimary: controlsCategoryHeader("Item usage"); break;
        case PlayerLeft: controlsCategoryHeader("Player movement"); break;
        case Quit: controlsCategoryHeader("Other"); break;
        }

        TableNextRow();
        TableNextColumn();

        TextUnformatted(k_keybindingInfo[i].desc);
        TableNextColumn();

        RealWorldKeyBindings binding = static_cast<RealWorldKeyBindings>(i);
        if (Button(re::toString(keybinder(binding)).data())) {
            keybinder().listenChangeBinding<MainMenuRoom, &MainMenuRoom::keybindCallback>(
                binding, *this
            );
            m_drawKeybindListeningPopup = true;
        }

        if (keybinder(binding) != k_keybindingInfo[i].defaultValue) {
            TableNextColumn();
            if (Button("Reset")) {
                keybinder().resetBinding(static_cast<RealWorldKeyBindings>(i));
            }
        }
        PopID();
    }

    if (m_drawKeybindListeningPopup) {
        glm::vec2 display = engine().windowDims();
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        SetNextWindowContentSize(keybindNoticeSize());
        SetNextWindowPos(display * 0.5f - keybindNoticeSize() * 0.5f);
        if (Begin("##listening", nullptr, ImGuiWindowFlags_NoDecoration)) {
            TextUnformatted(k_keybindNotice);
        }
        End();
        PopStyleVar();
    }

    EndTable();
}

} // namespace rw