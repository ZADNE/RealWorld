/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/textures/Texture.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

#include <RealWorld/drawing/WorldDrawerPushConstants.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

 /**
  * @brief Renders tiles of the world
 */
class TileDrawer {
public:

    TileDrawer(WorldDrawerPushConstants& pushConstants, const glm::uvec2& viewSizeTi);

    void draw(const vk::CommandBuffer& commandBuffer, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);

private:

    WorldDrawerPushConstants& m_pushConstants;
    RE::Texture m_blockAtlasTex{{.file = "blockAtlas"}};
    RE::Texture m_wallAtlasTex{{.file = "wallAtlas"}};

    RE::Pipeline m_drawTilesPl{{}, {{}, vk::PrimitiveTopology::eTriangleStrip}, {.vert = drawTiles_vert, .frag = drawColor_frag}};
};
