﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>
#include <RealEngine/rendering/buffers/TypedBuffer.hpp>

#include <RealWorld/shaders/drawing.hpp>

 /**
  * @brief Renders tiles of the world
 */
class TileDrawer {
public:
	TileDrawer(const glm::uvec2& viewSizeTi);

	void draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);
private:
	RE::TexturePtr m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	RE::TexturePtr m_wallAtlasTex = RE::RM::getTexture("wallAtlas");

	RE::ShaderProgram m_drawTilesShd{ {.vert = drawTiles_vert, .frag = colorDraw_frag} };
};