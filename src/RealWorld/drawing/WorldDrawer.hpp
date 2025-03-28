﻿/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/drawing/MinimapDrawer.hpp>
#include <RealWorld/drawing/ShadowDrawer.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
#include <RealWorld/drawing/shaders/WorldDrawingPC_glsl.hpp>

namespace rw {

/**
 * @brief Renders the world (i.e. tiles, shadows, minimap)
 */
class WorldDrawer {
public:
    WorldDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::uvec2 viewSizePx,
        glm::uint maxNumberOfExternalLights
    );

    void setTarget(const re::Texture& worldTex, glm::ivec2 worldTexSizeTi, float seed);
    void resizeView(glm::uvec2 viewSizePx);

    struct ViewEnvelope {
        glm::vec2 botLeftTi;
        glm::vec2 topRightTi;
    };
    ViewEnvelope setPosition(glm::vec2 botLeftPx);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
     */
    void beginStep(const re::CommandBuffer& cb, float timeDay);

    /**
     * @brief Adds an external light into the world. Must be used between
     * beginStep() and endStep()
     */
    void addExternalLight(glm::ivec2 posPx, re::Color col);

    /**
     * @brief External lights have to be added between beginStep() and endStep()
     */
    void endStep(const re::CommandBuffer& cb);

    void drawTiles(const re::CommandBuffer& cb);

    void drawShadows(const re::CommandBuffer& cb);

    void drawMinimap(const re::CommandBuffer& cb);

private:
    glm::vec2 m_botLeftPx{};  ///< Bottom-left corner of the view
    glm::ivec2 m_botLeftTi{}; ///< Bottom-left corner of the view in tiles

    glm::uvec2 m_viewSizeTi{};
    glm::uvec2 viewSizeTi(glm::vec2 viewSizePx) const;

    float m_skyLightPower = 1.0f;
    float timeToSkyLightPower(float timeDay) const;

    glsl::WorldDrawingPC m_pc{};

    TileDrawer m_tileDrawer;
    ShadowDrawer m_shadowDrawer;
    MinimapDrawer m_minimapDawer;
};

} // namespace rw
