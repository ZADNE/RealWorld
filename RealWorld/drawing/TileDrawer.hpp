/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>
#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/textures/TextureShaped.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

 /**
  * @brief Renders tiles of the world
  * @note Also renders minimap of the world
 */
class TileDrawer {
public:

    TileDrawer(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi);

    void setTarget(const re::Texture& worldTexture, const glm::ivec2& worldTexSize);

    void resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi);

    void drawTiles(const vk::CommandBuffer& commandBuffer, const glm::vec2& botLeftPx);

    void drawMinimap(const vk::CommandBuffer& commandBuffer);

private:

    re::TextureShaped m_blockAtlasTex{{.file = "blockAtlas"}};
    re::TextureShaped m_wallAtlasTex{{.file = "wallAtlas"}};

    struct PushConstants {
        glm::mat4 viewMat;
        glm::ivec2 worldTexMask = glm::ivec2(1, 1);
        glm::ivec2 viewSizeTi;
        glm::vec2 botLeftPxModTilePx;
        glm::ivec2 botLeftTi;
        glm::vec2 minimapOffset;
        glm::vec2 minimapSize;
    };

    PushConstants m_pushConstants;
    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet m_descriptorSet{m_pipelineLayout, 0u};
    re::Pipeline m_drawTilesPl;
    re::Pipeline m_drawMinimapPl;
};
