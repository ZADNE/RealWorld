/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/TileDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>

#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

TileDrawer::TileDrawer(const glm::uvec2& viewSizePx, const glm::ivec2& viewSizeTi):
    m_pipelineLayout({}, RE::PipelineLayoutDescription{
        .bindings = {{
            {0u, eCombinedImageSampler, 1u, eVertex | eFragment},   //worldTexture
            {1u, eCombinedImageSampler, 1u, eVertex | eFragment},   //blockAtlas
            {2u, eCombinedImageSampler, 1u, eVertex | eFragment}    //wallAtlas
        }},
        .ranges = {vk::PushConstantRange{eVertex | eFragment, 0u, sizeof(PushConstants)}}
    }),
    m_drawTilesPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *m_pipelineLayout,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, RE::PipelineGraphicsSources{
            .vert = drawTiles_vert,
            .frag = drawColor_frag
        }
    ),
    m_drawMinimapPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *m_pipelineLayout,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, RE::PipelineGraphicsSources{
            .vert = drawMinimap_vert,
            .frag = drawMinimap_frag
        }
    ) {
    m_descriptorSet.write(vk::DescriptorType::eCombinedImageSampler, 1u, 0u, m_blockAtlasTex);
    m_descriptorSet.write(vk::DescriptorType::eCombinedImageSampler, 2u, 0u, m_wallAtlasTex);

    resizeView(viewSizePx, viewSizeTi);
}

void TileDrawer::setTarget(const RE::Texture& worldTexture, const glm::ivec2& worldTexSize) {
    m_descriptorSet.write(vk::DescriptorType::eCombinedImageSampler, 0u, 0u, worldTexture);
    m_pushConstants.worldTexMask = worldTexSize - 1;
}

void TileDrawer::resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi) {
    m_pushConstants.viewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_pushConstants.viewSizeTi = viewSizeTi;
    float minimapDim = glm::min(viewSizePx.x * 0.875f, viewSizePx.y * 0.875f);
    m_pushConstants.minimapSize = {minimapDim, minimapDim};
    m_pushConstants.minimapOffset = viewSizePx * 0.5f - m_pushConstants.minimapSize * 0.5f;
}

void TileDrawer::drawTiles(const vk::CommandBuffer& commandBuffer, const glm::vec2& botLeftPx) {
    m_pushConstants.botLeftPxModTilePx = glm::mod(botLeftPx, TILEPx);
    m_pushConstants.botLeftTi = glm::ivec2(pxToTi(botLeftPx));
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    commandBuffer.pushConstants<PushConstants>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants);
    commandBuffer.draw(4u, m_pushConstants.viewSizeTi.x * m_pushConstants.viewSizeTi.y, 0u, 0u);
}

void TileDrawer::drawMinimap(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    commandBuffer.pushConstants<PushConstants>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pushConstants);
    commandBuffer.draw(4u, 1u, 0u, 0u);
}
