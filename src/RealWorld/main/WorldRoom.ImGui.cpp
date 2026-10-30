/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

#include <ImGui/imgui.h>

#include <RealWorld/save/WorldSaveLoader.hpp>

namespace rw {

void WorldRoom::drawDeveloperGui() {
    ImGui::PushFont(m_monospaceFont, m_monospaceFont->LegacySize);
    if (ImGui::Begin("Developer's GUI", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string fpsFrameTimeStr = std::format(
            "FPS: {}\nMax FT: {} us", engine().framesPerSecond(),
            duration_cast<std::chrono::microseconds>(engine().maxFrameTime()).count()
        );
        ImGui::TextUnformatted(fpsFrameTimeStr.c_str());
        if (ImGui::BeginTabBar("DebugTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Basic")) {
                ImGui::Text(
                    "Player posPx [%.0f, %.0f]", m_player.centerPx().x,
                    m_player.centerPx().y
                );
                ImGui::Text(
                    "Cursor posPx [%.0f, %.0f]", m_worldView.cursorRel().x,
                    m_worldView.cursorRel().y
                );
                ImGui::TextUnformatted("Minimap:");
                ImGui::SameLine();
                ImGui::Checkbox("##minimap", &m_minimap);
                ImGui::TextUnformatted("Shadows:");
                ImGui::SameLine();
                ImGui::Checkbox("##shadows", &m_shadows);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Light")) {
                ImGui::TextUnformatted("Stop daytime:");
                ImGui::SameLine();
                ImGui::Checkbox("##stopDaytime", &m_stopDaytime);
                ImGui::SameLine();
                float hours   = glm::floor(glm::fract(m_timeD) * 24.0f);
                float minutes = glm::floor(glm::fract(m_timeD * 24.0f) * 60.0f);
                ImGui::Text("%02.f:%02.f", hours, minutes);
                ImGui::SetNextItemWidth(800.0f);
                ImGui::SliderFloat(
                    "##time", &m_timeD, 0.0f, std::nextafter(1.0f, 0.0f), ""
                );
                drawDebugLightGui("Player", m_playerLight);
                drawDebugLightGui("Cursor", m_cursorLight);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("World gen")) {
                ImGui::DragInt(
                    "Seed", &m_newWorldSeed, 1.0f, 0, WorldSaveLoader::k_seedSafeMax
                );
                if (ImGui::Button("Randomize seed")) {
                    m_newWorldSeed = m_rndDevice() & WorldSaveLoader::k_seedSafeMax;
                }
                if (ImGui::Button("To the world!")) {
                    auto save = WorldSaveLoader::createWorld(
                        m_world.worldName(), m_newWorldSeed
                    );
                    save.metadata.timeD = m_timeD;
                    adoptWorld(save);
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopFont();
    engine().mainRenderPassDrawImGui();
}

void WorldRoom::drawDebugLightGui(const char* name, DebugLight& light) {
    ImGui::ColorEdit3(
        std::format("{} light", name).c_str(), &light.color.r, ImGuiColorEditFlags_Float
    );
    ImGui::SameLine();
    ImGui::DragFloat(
        std::format("Intensity##{}", name).c_str(), &light.intensity, 0.01f,
        0.0f, 16.0f, "%.5f", ImGuiSliderFlags_Logarithmic
    );
    constexpr static std::array<const char*, 3> k_posStr{
        "Exact", "Circling", "Offset"
    };
    ImGui::Combo(
        std::format("Position##{}", name).c_str(),
        reinterpret_cast<int*>(&light.relPos), k_posStr.data(), k_posStr.size()
    );
    if (light.relPos == DebugRelLightPos::Offset) {
        ImGui::SameLine();
        ImGui::SliderFloat2(
            std::format("Offset##{}", name).c_str(), &light.offsetPx.x, 0.0f,
            tiToPx(glsl::k_lightMaxCellTi)
        );
    }
}

} // namespace rw
