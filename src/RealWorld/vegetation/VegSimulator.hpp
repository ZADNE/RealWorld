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
#include <RealEngine/graphics/textures/Texture.hpp>

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
    void unrasterizeVegetation(const re::CommandBuffer& cmdBuf);

    /**
     * @brief   Adds simulated branches to the world texture
     */
    void rasterizeVegetation(const re::CommandBuffer& cmdBuf);

    struct VegStorage {
        const re::Buffer& branchBuf;
    };

    VegStorage adoptSave(const re::Texture& worldTex, glm::ivec2 worldTexSizeCh);

    /**
     * @brief   Returns index to the double buffered part of the branch buffer
     * is currently for writing (swaps each step)
     */
    glm::uint writeBuf() const { return 1 - m_vegDynamicsPC.readBuf; }

private:
    struct VegDynamicsPC {
        glm::mat4 mvpMat;
        glm::vec2 worldTexSizeTi;
        float     timeSec = static_cast<float>(time(nullptr) & 0xFFFF);
        glm::uint readBuf = 0;
    } m_vegDynamicsPC;

    re::PipelineLayout m_pipelineLayout;
    re::RenderPass     m_unrasterizationRenderPass;
    re::Framebuffer    m_unrasterizationFramebuffer;
    re::RenderPass     m_rasterizationRenderPass;
    re::Framebuffer    m_rasterizationFramebuffer;

    re::Pipeline m_unrasterizeBranchesPl{
        {.topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .pipelineLayout     = *m_pipelineLayout,
         .renderPass         = *m_unrasterizationRenderPass,
         .debugName          = "rw::VegSimulator::unrasterizeBranches"},
        {.vert = unrasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .geom = duplicateBranches_geom,
         .frag = unrasterizeBranches_frag}};
    re::Pipeline m_rasterizeBranchesPl{
        {.topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .pipelineLayout     = *m_pipelineLayout,
         .renderPass         = *m_rasterizationRenderPass,
         .debugName          = "rw::VegSimulator::rasterizeBranches"},
        {.vert = rasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .geom = duplicateBranches_geom,
         .frag = rasterizeBranches_frag}};
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::VegSimulator::descriptorSet"}};
    re::Buffer        m_branchBuf;
    glm::uvec2        m_worldTexSizeTi{};

    void beginWorldTextureRenderPass(
        const re::CommandBuffer& cmdBuf,
        const vk::RenderPass&    renderPass,
        const vk::Framebuffer&   framebuffer
    ) const;
};

} // namespace rw
