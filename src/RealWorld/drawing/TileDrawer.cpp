/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/MinimapLayout.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;

namespace rw {

TileDrawer::TileDrawer(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi)
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eVertex | eFragment}, // worldTexture
                  {1u, eCombinedImageSampler, 1u, eVertex | eFragment}, // blockAtlas
                  {2u, eCombinedImageSampler, 1u, eVertex | eFragment} // wallAtlas
              }},
              .ranges   = {vk::PushConstantRange{
                  eVertex | eFragment, 0u, sizeof(PushConstants)}}}
      )
    , m_drawTilesPl(
          re::PipelineGraphicsCreateInfo{
              .topology       = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout = *m_pipelineLayout,
              .debugName      = "rw::TileDrawer::drawTiles"},
          re::PipelineGraphicsSources{.vert = drawTiles_vert, .frag = drawColor_frag}
      )
    , m_drawMinimapPl(
          re::PipelineGraphicsCreateInfo{
              .topology       = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout = *m_pipelineLayout,
              .debugName      = "rw::TileDrawer::drawMinimap"},
          re::PipelineGraphicsSources{.vert = drawMinimap_vert, .frag = drawMinimap_frag}
      ) {
    m_descriptorSet.write(
        eCombinedImageSampler, 1u, 0u, m_blockAtlasTex, eReadOnlyOptimal
    );
    m_descriptorSet.write(eCombinedImageSampler, 2u, 0u, m_wallAtlasTex, eReadOnlyOptimal);

    resizeView(viewSizePx, viewSizeTi);
}

void TileDrawer::setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize) {
    m_descriptorSet.write(eCombinedImageSampler, 0u, 0u, worldTexture, eReadOnlyOptimal);
    m_pushConstants.worldTexMask = worldTexSize - 1;
    glm::vec2 viewSizePx =
        glm::vec2(2.0f, 2.0f) /
        glm::vec2(m_pushConstants.viewMat[0][0], m_pushConstants.viewMat[1][1]);
    resizeView(viewSizePx, m_pushConstants.viewSizeTi);
}

void TileDrawer::resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi) {
    m_pushConstants.viewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_pushConstants.viewSizeTi = viewSizeTi;

    auto layout = minimapLayout(m_pushConstants.worldTexMask + 1, viewSizePx);
    m_pushConstants.minimapOffset = layout.offsetPx;
    m_pushConstants.minimapSize   = layout.sizePx;
}

void TileDrawer::drawTiles(const re::CommandBuffer& cb, glm::vec2 botLeftPx) {
    m_pushConstants.botLeftPxModTilePx = glm::mod(botLeftPx, TilePx);
    m_pushConstants.botLeftTi          = glm::ivec2(pxToTi(botLeftPx));
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    cb->pushConstants<PushConstants>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants
    );
    cb->draw(
        4u, m_pushConstants.viewSizeTi.x * m_pushConstants.viewSizeTi.y, 0u, 0u
    );
}

void TileDrawer::drawMinimap(const re::CommandBuffer& cb) {
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    cb->pushConstants<PushConstants>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants
    );
    cb->draw(4u, 1u, 0u, 0u);
}

} // namespace rw
