/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/vertices/VertexArray.hpp>
#include <RealEngine/rendering/textures/Texture.hpp>

#include <RealWorld/drawing/shaders/AllShaders.hpp>

/**
 * @brief Renders tiles of the world
*/
template<RE::Renderer R>
class TileDrawer {
public:

    TileDrawer(const glm::uvec2& viewSizeTi);

    void draw(const RE::VertexArray<R>& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);

private:

    RE::Texture<R> m_blockAtlasTex{{.file = "blockAtlas"}};
    RE::Texture<R> m_wallAtlasTex{{.file = "wallAtlas"}};

    RE::ShaderProgram<R> m_drawTilesShd{ {.vert = drawTiles_vert, .frag = drawColor_frag} };
};
