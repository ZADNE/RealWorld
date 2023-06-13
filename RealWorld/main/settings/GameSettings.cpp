/*!
 *  @author    Dubsky Tomas
 */
#include <filesystem>
#include <fstream>

#include <RealWorld/main/settings/GameSettings.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace rw {

const std::string SETTINGS_FILENAME = "game_settings.json";

GameSettings::GameSettings() {
    std::ifstream i(SETTINGS_FILENAME);
    json          j;
    try {
        i >> j;
        m_worldTexSize.x = j["world_tex_size"]["width"].get<int>();
        m_worldTexSize.y = j["world_tex_size"]["height"].get<int>();
    } catch (...) {
        // Settings either don't exist or are currupted
        i.close();
        if (std::filesystem::exists(SETTINGS_FILENAME)) {
            // Back up previous settings
            std::filesystem::rename(
                SETTINGS_FILENAME, SETTINGS_FILENAME + ".backup"
            );
        }
        reset(); // Reset settings to default values
        save();
    }
}

void GameSettings::reset() {
    m_worldTexSize = glm::ivec2(32, 32);
}

void GameSettings::save() {
    nlohmann::ordered_json j = {
        {"world_tex_size",
         {{"width", m_worldTexSize.x}, {"height", m_worldTexSize.y}}}};

    std::ofstream o(SETTINGS_FILENAME, std::ofstream::trunc);
    o << j.dump(2);
    o.close();
}

} // namespace rw
