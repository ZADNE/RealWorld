/**
 *  @author    Dubsky Tomas
 */
#pragma once

#include <string>

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Specifies settings specific to RealWorld game.
 */
class GameSettings {
public:
    /**
     * @brief Constructs settings from file that was saved before.
     * @details If the file cannot be loaded, settings are constructed with the
     * default values.
     */
    GameSettings();

    /**
     * @brief Save current settings to a file.
     * @details Saved settings can be loaded by default contructor.
     */
    void save();

    /**
     * @brief Resets settings to default values.
     */
    void reset();

    glm::ivec2 worldTexSize() const { return m_worldTexSize; }

    void setWorldTexSize(glm::ivec2 worldTexSize) {
        m_worldTexSize = worldTexSize;
    }

private:
    glm::ivec2 m_worldTexSize{};
};

} // namespace rw
