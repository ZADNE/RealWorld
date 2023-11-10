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

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/vegetation/Branch.hpp>
#include <RealWorld/vegetation/shaders/AllShaders.hpp>

namespace rw {

#pragma warning(push)
#pragma warning(disable : 4200)
struct BranchSB {
    // Double-buffered params
    glm::vec2 absPosTi[2][k_maxBranchCount];
    float     absAngNorm[2][k_maxBranchCount];

    // Single-buffered params
    glm::uint parentOffset15wallType31[k_maxBranchCount];
    float     relRestAngNorm[k_maxBranchCount];
    float     angVel[k_maxBranchCount];
    float     radiusTi[k_maxBranchCount];
    float     lengthTi[k_maxBranchCount];
    glm::vec2 densityStiffness[k_maxBranchCount];
    uint8_t   raster[k_maxBranchCount][k_branchRasterSpace];

    // Footer
    glm::uint vertexCount;
    glm::uint instanceCount;
    glm::uint firstVertex;
    glm::uint firstInstance;
};
#pragma warning(pop)

/**
 * @brief   Simulates swaying and growth of vegetation
 * @details Rasterizes and unrasterizes the vegetation to the world texture.
 */
class VegSimulator {
public:
    VegSimulator();

    void step(const vk::CommandBuffer& commandBuffer);

    struct VegStorage {
        const re::Buffer& vegBuf;
        const re::Buffer& branchBuf;
    };

    VegStorage adoptSave(const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh);

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
    re::RenderPass     m_rasterizationRenderPass;

    re::Pipeline m_unrasterizeBranchesPl{
        {.topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .pipelineLayout     = *m_pipelineLayout,
         .renderPass         = *m_rasterizationRenderPass},
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
         .subpassIndex       = 1},
        {.vert = rasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .geom = duplicateBranches_geom,
         .frag = rasterizeBranches_frag}};
    re::DescriptorSet  m_descriptorSet{m_pipelineLayout.descriptorSetLayout(0)};
    re::Buffer         m_vegBuf;
    re::Buffer         m_branchBuf;
    re::Framebuffer    m_framebuffer;
    const re::Texture* m_worldTex{};
    glm::uvec2         m_worldTexSizeTi{};
};

} // namespace rw
