/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/pipelines/Vertex.hpp>

#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/drawing/WorldDrawerPushConstants.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
 //#include <RealWorld/drawing/ShadowDrawer.hpp>
 //#include <RealWorld/drawing/MinimapDrawer.hpp>

 /**
 * @brief Renders the world (i.e. tiles, shadows, minimap)
 */
class WorldDrawer {
public:

    WorldDrawer(const glm::uvec2& viewSizePx);

    void setTarget(const glm::ivec2& worldTexSize);
    void resizeView(const glm::uvec2& viewSizePx);

    struct ViewEnvelope {
        glm::vec2 botLeftTi;
        glm::vec2 topRightTi;
    };
    ViewEnvelope setPosition(const glm::vec2& botLeftPx);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
    */
    void beginStep();

    /**
     * @brief Adds an external light into the world. Must be used between beginStep() and endStep()
    */
    void addExternalLight(const glm::ivec2& posPx, RE::Color col);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
    */
    void endStep();

    void drawTiles(const vk::CommandBuffer& commandBuffer);

    void shouldDrawShadows(bool should) { m_drawShadows = should; }
    void drawShadows(const vk::CommandBuffer& commandBuffer);

    void shouldDrawMinimap(bool should) { m_drawMinimap = should; }
    void drawMinimap(const vk::CommandBuffer& commandBuffer);

private:

    void updateViewSizeDependentUniforms();

    glm::vec2 m_botLeftPx;//Bottom-left corner of the view
    glm::ivec2 m_botLeftTi;//Bottom-left corner of the view in tiles

    glm::vec2 m_viewSizePx;
    glm::uvec2 m_viewSizeTi;
    glm::uvec2 viewSizeTi(const glm::vec2& viewSizePx) const;

    glm::ivec2 m_worldTexSize;

    bool m_drawShadows = true;
    bool m_drawMinimap = false;

    WorldDrawerPushConstants m_pushConstants;
    RE::PipelineLayout m_pipelineLayout{{}, {.vert = drawTiles_vert}};

    TileDrawer m_tileDrawer;
    //ShadowDrawer m_shadowDrawer;
    //MinimapDrawer m_minimapDrawer;
};
