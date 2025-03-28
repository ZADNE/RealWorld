﻿/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <string>

#include <ImGui/imgui.h>
#include <glm/vec2.hpp>

#include <RealEngine/window/WindowSubsystems.hpp>

#include <RealWorld/constants/World.hpp>

namespace rw {

constexpr std::array k_resolutions = {
    glm::ivec2{1280, 1024}, glm::ivec2{1360, 768},  glm::ivec2{1366, 768},
    glm::ivec2{1440, 900},  glm::ivec2{1600, 900},  glm::ivec2{1680, 1050},
    glm::ivec2{1920, 1080}, glm::ivec2{2560, 1080}, glm::ivec2{2560, 1440},
    glm::ivec2{3440, 1440}, glm::ivec2{3840, 2160}
};

constexpr std::array k_worldTexSizes = {
    k_minWorldTexSizeCh, glm::ivec2{32, 16}, glm::ivec2{32, 32},
    glm::ivec2{64, 32}, k_maxWorldTexSizeCh
};

inline std::string ivec2ToString(glm::ivec2 vec) {
    return std::to_string(vec.x) + "x" + std::to_string(vec.y);
}

template<typename Container, typename ToStringConvertor = std::identity>
bool comboSelect(
    const Container& combos, const char* label, float width,
    typename Container::const_iterator& selected,
    ToStringConvertor toString = std::identity{}
) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    bool changedSelection   = false;
    std::string hiddenlabel = std::string("##") + label;
    if (ImGui::BeginCombo(
            hiddenlabel.c_str(),
            selected != combos.end() ? toString(*selected).c_str() : ""
        )) {
        for (auto it = combos.begin(); it != combos.end(); ++it) {
            if (ImGui::Selectable(toString(*it).c_str(), it == selected)) {
                selected         = it;
                changedSelection = true;
            }
        }
        ImGui::EndCombo();
    }
    return changedSelection;
}

} // namespace rw
