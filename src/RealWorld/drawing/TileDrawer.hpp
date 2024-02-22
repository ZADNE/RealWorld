/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/textures/TextureShaped.hpp>

namespace rw {

/**
 * @brief Renders tiles and minimap of the world
 */
class TileDrawer {
public:
    TileDrawer(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi);

    void setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize);

    void resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi);

    void drawTiles(const re::CommandBuffer& cb, glm::vec2 botLeftPx);

    void drawMinimap(const re::CommandBuffer& cb);

private:
    re::TextureShaped m_blockAtlasTex{re::TextureSeed{"blockAtlas"}};
    re::TextureShaped m_wallAtlasTex{re::TextureSeed{"wallAtlas"}};

    struct PushConstants {
        glm::mat4  viewMat;
        glm::ivec2 worldTexMask = glm::ivec2(1, 1);
        glm::ivec2 viewSizeTi;
        glm::vec2  botLeftPxModTilePx;
        glm::ivec2 botLeftTi;
        glm::vec2  minimapOffset;
        glm::vec2  minimapSize;
    } m_pushConstants;

    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet  m_descriptorSet{re::DescriptorSetCreateInfo{
         .layout    = m_pipelineLayout.descriptorSetLayout(0),
         .debugName = "rw::TileDrawer::descriptorSet"}};
    re::Pipeline       m_drawTilesPl;

    // Minimap
    re::Pipeline m_drawMinimapPl;
};

} // namespace rw