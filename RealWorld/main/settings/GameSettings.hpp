/*!
 *  @author    Dubsky Tomas
 */
#pragma once

#include <string>

#include <glm/vec2.hpp>

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

    const glm::ivec2& activeChunksArea() const { return m_activeChunksArea; }

    void setActiveChunksArea(const glm::ivec2& activeChunksArea) {
        m_activeChunksArea = activeChunksArea;
    }

private:
    glm::ivec2 m_activeChunksArea;
};
