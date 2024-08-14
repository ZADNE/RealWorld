/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/drawing/shaders/AllShaders.hpp>
#include <RealWorld/simulation/objects/DroppedTilesSB.hpp>

namespace rw {

/**
 * @brief Draw dropped tiles
 */
class DroppedTilesDrawer {
public:
    DroppedTilesDrawer(
        re::RenderPassSubpass renderPassSubpass, const re::Buffer& droppedTilesBuf
    );

    void draw(
        const re::CommandBuffer& cb, const glm::mat4& mvpMat, float timeSec,
        float interpFactor
    );

private:
    re::SharedTexture m_blockAtlasTex =
        re::RM::texture(re::TextureSeed{"blockAtlas"});
    re::SharedTexture m_wallAtlasTex =
        re::RM::texture(re::TextureSeed{"wallAtlas"});

    struct DroppedTilesDrawingPC {
        glm::mat4 mvpMat;
        float blinkState;
        float interpFactor;
    };

    const re::Buffer& m_droppedTilesBuf;
    re::PipelineLayout m_pipelineLayout{
        {},
        re::PipelineLayoutDescription{
            .bindings =
                {{{0, vk::DescriptorType::eCombinedImageSampler, 2,
                   vk::ShaderStageFlagBits::eGeometry}}},
            .ranges = {vk::PushConstantRange{
                vk::ShaderStageFlagBits::eGeometry, 0u, sizeof(DroppedTilesDrawingPC)
            }}
        }
    };
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::DroppedTilesDrawer::descriptorSet"
    }};
    re::Pipeline m_drawDroppedTilesPl;
};

} // namespace rw
