/*!
 *  @author    Dubsky Tomas
 */

#include <array>
#include <string>

#include <glm/vec2.hpp>

#include <ImGui/imgui.h>


constexpr std::array RESOLUTIONS = {
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

constexpr std::array ACTIVE_CHUNKS_AREAS = {
	glm::ivec2{8, 8},
	glm::ivec2{16, 8},
	glm::ivec2{16, 16},
	glm::ivec2{32, 16},
	glm::ivec2{32, 32},
	glm::ivec2{64, 32},
	glm::ivec2{64, 64}
};

inline constexpr std::string ivec2ToString(const glm::ivec2& vec) {
	return std::to_string(vec.x) + "x" + std::to_string(vec.y);
}

template<std::array combos>
bool ivec2ComboSelect(const char* label, float width, size_t& selected) {
	ImGui::TextUnformatted(label); ImGui::SameLine();
	ImGui::SetNextItemWidth(width);
	bool changedSelection = false;
	std::string hiddenlabel = std::string("##") + label;
	if (ImGui::BeginCombo(hiddenlabel.c_str(), ivec2ToString(combos[selected]).c_str())) {
		for (size_t i = 0; i < combos.size(); ++i) {
			if (ImGui::Selectable(ivec2ToString(combos[i]).c_str(), i == selected)) {
				selected = i;
				changedSelection = true;
			}
		}
		ImGui::EndCombo();
	}
	return changedSelection;
}
