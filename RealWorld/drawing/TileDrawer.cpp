/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;

TileDrawer::TileDrawer(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi)
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
              .pipelineLayout = *m_pipelineLayout,
              .topology       = vk::PrimitiveTopology::eTriangleStrip},
          re::PipelineGraphicsSources{.vert = drawTiles_vert, .frag = drawColor_frag}
      )
    , m_drawMinimapPl(
          re::PipelineGraphicsCreateInfo{
              .pipelineLayout = *m_pipelineLayout,
              .topology       = vk::PrimitiveTopology::eTriangleStrip},
          re::PipelineGraphicsSources{.vert = drawMinimap_vert, .frag = drawMinimap_frag}
      ) {
    m_descriptorSet.write(
        eCombinedImageSampler, 1u, 0u, m_blockAtlasTex, eReadOnlyOptimal
    );
    m_descriptorSet.write(eCombinedImageSampler, 2u, 0u, m_wallAtlasTex, eReadOnlyOptimal);

    resizeView(viewSizePx, viewSizeTi);
}

void TileDrawer::setTarget(
    const re::Texture& worldTexture, const glm::ivec2& worldTexSize
) {
    m_descriptorSet.write(eCombinedImageSampler, 0u, 0u, worldTexture, eReadOnlyOptimal);
    m_pushConstants.worldTexMask = worldTexSize - 1;
    glm::vec2 viewSizePx =
        glm::vec2(2.0f, 2.0f) /
        glm::vec2(m_pushConstants.viewMat[0][0], m_pushConstants.viewMat[1][1]);
    resizeView(viewSizePx, m_pushConstants.viewSizeTi);
}

void TileDrawer::resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi) {
    m_pushConstants.viewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_pushConstants.viewSizeTi  = viewSizeTi;
    glm::vec2 worldTexSize      = m_pushConstants.worldTexMask + 1;
    float     worldTexSizeRatio = worldTexSize.x / worldTexSize.y;
    if (worldTexSizeRatio > 1.0f) {
        float longerDim = viewSizePx.x - 100.0f;
        m_pushConstants.minimapSize =
            glm::vec2(longerDim, longerDim / worldTexSizeRatio);
    } else if (worldTexSizeRatio < 1.0f) {
        float longerDim = viewSizePx.y - 100.0f;
        m_pushConstants.minimapSize =
            glm::vec2(longerDim * worldTexSizeRatio, longerDim);
    } else {
        float longerDim = glm::min(viewSizePx.x - 100.0f, viewSizePx.y - 100.0f);
        m_pushConstants.minimapSize =
            glm::vec2(longerDim * worldTexSizeRatio, longerDim);
    }
    m_pushConstants.minimapOffset = viewSizePx * 0.5f -
                                    m_pushConstants.minimapSize * 0.5f;
}

void TileDrawer::drawTiles(
    const vk::CommandBuffer& commandBuffer, const glm::vec2& botLeftPx
) {
    m_pushConstants.botLeftPxModTilePx = glm::mod(botLeftPx, TilePx);
    m_pushConstants.botLeftTi          = glm::ivec2(pxToTi(botLeftPx));
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    commandBuffer.pushConstants<PushConstants>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants
    );
    commandBuffer.draw(
        4u, m_pushConstants.viewSizeTi.x * m_pushConstants.viewSizeTi.y, 0u, 0u
    );
}

void TileDrawer::drawMinimap(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    commandBuffer.pushConstants<PushConstants>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants
    );
    commandBuffer.draw(4u, 1u, 0u, 0u);
}
