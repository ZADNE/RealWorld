/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/TileDrawer.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/buffers.hpp>


TileDrawer::TileDrawer(const glm::uvec2& viewSizeTi) {
    //Bind textures to their reserved texture units
    m_blockAtlasTex->bind(TEX_UNIT_BLOCK_ATLAS);
    m_wallAtlasTex->bind(TEX_UNIT_WALL_ATLAS);

    //Init shader
    m_drawTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
}

void TileDrawer::draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
    vao.bind();
    m_drawTilesShd.use();
    m_drawTilesShd.setUniform(LOC_POSITIONPx, glm::mod(botLeftPx, TILEPx));
    m_drawTilesShd.setUniform(LOC_POSITIONTi, glm::ivec2(pxToTi(botLeftPx)));
    vao.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
    m_drawTilesShd.unuse();
    vao.unbind();
}