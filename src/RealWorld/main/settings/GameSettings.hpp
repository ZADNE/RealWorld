/*!
 *  @author    Dubsky Tomas
 */
#pragma once

#include <string>

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Specify settings specific to RealWorld game.
 */
class GameSettings {
public:
    /**
     * @brief Constructs settings from file that was saved before.
     * @detailsIf the file cannot be loaded, settings are constructed with the
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

    const glm::ivec2& worldTexSize() const { return m_worldTexSize; }

    void setWorldTexSize(const glm::ivec2& worldTexSize) {
        m_worldTexSize = worldTexSize;
    }

private:
    glm::ivec2 m_worldTexSize;
};

} // namespace rw
