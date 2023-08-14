/*!
 *  @author    Dubsky Tomas
 */
#include <filesystem>
#include <fstream>

#include <RealWorld/main/settings/GameSettings.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace rw {

const std::string k_settingsFilename = "game_settings.json";

GameSettings::GameSettings() {
    std::ifstream i(k_settingsFilename);
    json          j;
    try {
        i >> j;
        m_worldTexSize.x = j["world_tex_size"]["width"].get<int>();
        m_worldTexSize.y = j["world_tex_size"]["height"].get<int>();
    } catch (...) {
        // Settings either don't exist or are currupted
        i.close();
        if (std::filesystem::exists(k_settingsFilename)) {
            // Back up previous settings
            std::filesystem::rename(
                k_settingsFilename, k_settingsFilename + ".backup"
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

    std::ofstream o(k_settingsFilename, std::ofstream::trunc);
    o << j.dump(2);
    o.close();
}

} // namespace rw
