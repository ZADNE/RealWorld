/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>
#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/textures/TextureShaped.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

#include <RealWorld/drawing/WorldDrawerPushConstants.hpp>

 /**
  * @brief Renders tiles of the world
 */
class TileDrawer {
public:

    TileDrawer(const RE::PipelineLayout& pipelineLayout, RE::DescriptorSet& descriptorSet);

    void draw(
        WorldDrawerPushConstants& pushConstants,
        const RE::PipelineLayout& pipelineLayout,
        const vk::CommandBuffer& commandBuffer,
        const glm::vec2& botLeftPx,
        const glm::uvec2& viewSizeTi
    );

private:

    RE::TextureShaped m_blockAtlasTex{{.file = "blockAtlas"}};
    RE::TextureShaped m_wallAtlasTex{{.file = "wallAtlas"}};

    RE::Pipeline m_drawTilesPl;
};
