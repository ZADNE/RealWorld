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

TileDrawer::TileDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx, WorldDrawingPC& pc
)
    : m_pc(pc)
    , m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eFragment}, // worldTexture
                  {1u, eCombinedImageSampler, 1u, eFragment}, // blockAtlas
                  {2u, eCombinedImageSampler, 1u, eFragment}  // wallAtlas
              }},
              .ranges   = {vk::PushConstantRange{
                  eVertex | eFragment, 0u, sizeof(WorldDrawingPC)
              }}
          }
      )
    , m_drawTilesPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .enableBlend       = false,
              .pipelineLayout    = *m_pipelineLayout,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::TileDrawer::drawTiles"
          },
          re::PipelineGraphicsSources{.vert = drawFullscreen_vert, .frag = drawTiles_frag}
      )
    , m_drawMinimapPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout    = *m_pipelineLayout,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::TileDrawer::drawMinimap"
          },
          re::PipelineGraphicsSources{.vert = drawMinimap_vert, .frag = drawMinimap_frag}
      ) {
    m_descriptorSet.write(
        eCombinedImageSampler, 1u, 0u, m_blockAtlasTex, eShaderReadOnlyOptimal
    );
    m_descriptorSet.write(
        eCombinedImageSampler, 2u, 0u, m_wallAtlasTex, eShaderReadOnlyOptimal
    );

    resizeView(viewSizePx);
}

void TileDrawer::setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize) {
    m_descriptorSet.write(
        eCombinedImageSampler, 0u, 0u, worldTexture, eShaderReadOnlyOptimal
    );
    m_pc.worldTexMask = worldTexSize - 1;
    glm::vec2 viewSizePx =
        glm::vec2(2.0f, 2.0f) /
        glm::vec2(m_pc.minimapViewMat[0][0], m_pc.minimapViewMat[1][1]);
    resizeView(viewSizePx);
}

void TileDrawer::resizeView(glm::vec2 viewSizePx) {
    m_pc.minimapViewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_viewSizePx        = viewSizePx;

    auto layout        = minimapLayout(m_pc.worldTexMask + 1, viewSizePx);
    m_pc.minimapOffset = layout.offsetPx;
    m_pc.minimapSize   = layout.sizePx;
}

void TileDrawer::drawTiles(const re::CommandBuffer& cb, glm::vec2 botLeftPx) {
    m_pc.uvRectSize   = m_viewSizePx;
    m_pc.uvRectOffset = glm::mod(botLeftPx, TilePx);
    m_pc.botLeftTi    = glm::ivec2(pxToTi(botLeftPx));
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    cb->pushConstants<WorldDrawingPC>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pc);
    cb->draw(3u, 1u, 0u, 0u);
}

void TileDrawer::drawMinimap(const re::CommandBuffer& cb) {
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    cb->pushConstants<WorldDrawingPC>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pc);
    cb->draw(4u, 1u, 0u, 0u);
}

} // namespace rw
