/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/drawing/ShadowDrawer.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>

/**
 * @brief Renders the world (i.e. tiles, shadows, minimap)
 */
class WorldDrawer {
public:
    WorldDrawer(const glm::uvec2& viewSizePx, glm::uint maxNumberOfExternalLights);

    void setTarget(const re::Texture& worldTexture, const glm::ivec2& worldTexSize);
    void resizeView(const glm::uvec2& viewSizePx);

    struct ViewEnvelope {
        glm::vec2 botLeftTi;
        glm::vec2 topRightTi;
    };
    ViewEnvelope setPosition(const glm::vec2& botLeftPx);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
     */
    void beginStep(const vk::CommandBuffer& commandBuffer);

    /**
     * @brief Adds an external light into the world. Must be used between
     * beginStep() and endStep()
     */
    void addExternalLight(const glm::ivec2& posPx, re::Color col);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
     */
    void endStep(const vk::CommandBuffer& commandBuffer);

    void drawTiles(const vk::CommandBuffer& commandBuffer);

    void drawShadows(const vk::CommandBuffer& commandBuffer);

    void drawMinimap(const vk::CommandBuffer& commandBuffer);

private:
    glm::vec2  m_botLeftPx; // Bottom-left corner of the view
    glm::ivec2 m_botLeftTi; // Bottom-left corner of the view in tiles

    glm::uvec2 m_viewSizeTi;
    glm::uvec2 viewSizeTi(const glm::vec2& viewSizePx) const;

    TileDrawer   m_tileDrawer;
    ShadowDrawer m_shadowDrawer;
};
