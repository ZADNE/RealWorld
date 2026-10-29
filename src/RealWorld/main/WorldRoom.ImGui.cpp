/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

#include <ImGui/imgui.h>

#include <RealWorld/save/WorldSaveLoader.hpp>

namespace rw {

void WorldRoom::drawDeveloperGui() {
    ImGui::PushFont(m_monospaceFont, m_monospaceFont->LegacySize);
    if (ImGui::Begin("##topLeftMenu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::BeginTabBar("DebugTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Basic")) {
                std::string fpsFrameTimeStr = std::format(
                    "FPS: {}\nMax FT: {} us", engine().framesPerSecond(),
                    duration_cast<std::chrono::microseconds>(engine().maxFrameTime())
                        .count()
                );
                ImGui::TextUnformatted(fpsFrameTimeStr.c_str());
                ImGui::Separator();
                ImGui::TextUnformatted("Minimap:");
                ImGui::SameLine();
                ImGui::Checkbox("##minimap", &m_minimap);
                ImGui::TextUnformatted("Shadows:");
                ImGui::SameLine();
                ImGui::Checkbox("##shadows", &m_shadows);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Time")) {
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
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("World gen")) {
                if (ImGui::Button("To random world")) {
                    auto save = WorldSaveLoader::createWorld(
                        m_world.worldName(), static_cast<int>(time(nullptr)) &
                                                 WorldSaveLoader::k_seedSafeMax
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

} // namespace rw
