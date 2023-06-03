/*!
 *  @author    Dubsky Tomas
 */
#include <filesystem>
#include <fstream>

#include <RealWorld/main/settings/GameSettings.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string SETTINGS_FILENAME = "game_settings.json";

GameSettings::GameSettings() {
    std::ifstream i(SETTINGS_FILENAME);
    json          j;
    try {
        i >> j;
        m_activeChunksArea.x = j["active_chunks_area"]["width"].get<int>();
        m_activeChunksArea.y = j["active_chunks_area"]["height"].get<int>();
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
    m_activeChunksArea = glm::ivec2(8, 8);
}

void GameSettings::save() {
    nlohmann::ordered_json j = {
        {"active_chunks_area",
         {{"width", m_activeChunksArea.x}, {"height", m_activeChunksArea.y}}}};

    std::ofstream o(SETTINGS_FILENAME, std::ofstream::trunc);
    o << j.dump(2);
    o.close();
}
