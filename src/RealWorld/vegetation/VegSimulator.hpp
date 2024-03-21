/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/output_control/Framebuffer.hpp>
#include <RealEngine/graphics/output_control/RenderPass.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>
#include <RealEngine/graphics/textures/ImageView.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/main/ActionCmdBuf.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/vegetation/shaders/AllShaders.hpp>

namespace rw {

/**
 * @brief   Simulates swaying and growth of vegetation
 * @details Rasterizes and unrasterizes the vegetation to the world texture.
 */
class VegSimulator {
public:
    VegSimulator();

    /**
     * @brief   Removes branches from the world texture
     */
    void unrasterizeVegetation(const ActionCmdBuf& acb);

    /**
     * @brief   Adds simulated branches to the world texture
     */
    void rasterizeVegetation(const ActionCmdBuf& acb, float timeSec);

    struct VegStorage {
        const re::Buffer& branchBuf;
        const re::Buffer& branchAllocRegBuf;
    };

    VegStorage adoptSave(const re::Texture& worldTex, glm::ivec2 worldTexSizeCh);

private:
    struct VegDynamicsPC {
        glm::mat4 mvpMat;
        glm::vec2 worldTexSizeTi;
        glm::ivec2 worldTexMaskTi;
        float timeSec;
        glm::uint readBuf;
    } m_vegDynamicsPC;

    re::PipelineLayout m_pipelineLayout;
    re::RenderPass m_rasterizationRenderPass;
    re::ImageView m_wallLayerImageView;
    re::Framebuffer m_rasterizationFramebuffer;

    re::Pipeline m_unrasterizeBranchesPl{
        {.topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .pipelineLayout     = *m_pipelineLayout,
         .renderPassSubpass  = m_rasterizationRenderPass.subpass(0),
         .debugName          = "rw::VegSimulator::unrasterizeBranches"},
        {.vert = unrasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .geom = duplicateBranches_geom,
         .frag = unrasterizeBranches_frag}
    };
    re::Pipeline m_rasterizeBranchesPl{
        {.topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .pipelineLayout     = *m_pipelineLayout,
         .renderPassSubpass  = m_rasterizationRenderPass.subpass(0),
         .debugName          = "rw::VegSimulator::rasterizeBranches"},
        {.vert = rasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .geom = duplicateBranches_geom,
         .frag = rasterizeBranches_frag}
    };
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::VegSimulator::descriptorSet"
    }};
    re::Buffer m_branchBuf;
    re::Buffer m_branchAllocRegBuf;
    glm::uvec2 m_worldTexSizeTi{};

    void beginWorldTextureRenderPass(const re::CommandBuffer& cb) const;
};

} // namespace rw
