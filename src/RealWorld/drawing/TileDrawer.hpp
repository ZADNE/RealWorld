/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/drawing/shaders/WorldDrawingPC_glsl.hpp>

namespace rw {

/**
 * @brief Renders tiles and minimap of the world
 */
class TileDrawer {
public:
    TileDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
        glsl::WorldDrawingPC& pc
    );

    void setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize, float seed);

    void resizeView(glm::vec2 viewSizePx);

    void drawTiles(const re::CommandBuffer& cb, glm::vec2 botLeftPx, float skyLight);

    void drawMinimap(const re::CommandBuffer& cb);

private:
    re::TextureShaped m_blockAtlasTex =
        re::RM::textureUnmanaged(textureID<"blockAtlas">());
    re::TextureShaped m_wallAtlasTex =
        re::RM::textureUnmanaged(textureID<"wallAtlas">());

    glm::vec2 m_viewSizePx{};

    glsl::WorldDrawingPC& m_pc;

    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::TileDrawer::descriptorSet"
    }};
    re::Pipeline m_drawTilesPl;
    float m_seed{};

    // Minimap
    re::Pipeline m_drawMinimapPl;
};

} // namespace rw
