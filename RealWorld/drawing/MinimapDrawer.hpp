/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

#include <RealWorld/drawing/WorldDrawerPushConstants.hpp>

/**
 * @brief Renders minimap of the world
*/
class MinimapDrawer {
public:

    MinimapDrawer(const RE::PipelineLayout& pipelineLayout);

    void setTarget(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx);
    void resizeView(const glm::ivec2& worldTexSize, const glm::uvec2& viewSizePx);

    void draw(const vk::CommandBuffer& commandBuffer);

private:

    RE::Pipeline m_drawMinimapPl;
};
