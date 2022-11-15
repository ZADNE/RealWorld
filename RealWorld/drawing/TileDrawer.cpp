/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/TileDrawer.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/drawing/WorldDrawerUniforms.hpp>

template<RE::Renderer R>
TileDrawer<R>::TileDrawer(const glm::uvec2& viewSizeTi) {
    //Bind textures to their reserved texture units
    m_blockAtlasTex.bind(TEX_UNIT_BLOCK_ATLAS);
    m_wallAtlasTex.bind(TEX_UNIT_WALL_ATLAS);

    //Init shader
    m_drawTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
}

template<RE::Renderer R>
void TileDrawer<R>::draw(const RE::BufferTyped<R>& uniformBuf, const RE::VertexArray<R>& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, botLeftPxModTilePx), glm::mod(botLeftPx, TILEPx));
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, botLeftTi), glm::ivec2(pxToTi(botLeftPx)));
    vao.bind();
    m_drawTilesShd.use();
    vao.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
    m_drawTilesShd.unuse();
    vao.unbind();
}

template class TileDrawer<RE::RendererVK13>;
template class TileDrawer<RE::RendererGL46>;