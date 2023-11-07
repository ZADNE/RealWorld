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
#include <RealWorld/vegetation/Branch.hpp>
#include <RealWorld/vegetation/shaders/AllShaders.hpp>

namespace rw {

struct BranchesSBHeader {
    uint32_t vertexCount    = 0;
    uint32_t instanceCount  = 1;
    uint32_t firstVertex    = 0;
    uint32_t firstInstance  = 0;
    int      maxBranchCount = 0;
    int      padding[7];
};

#pragma warning(push)
#pragma warning(disable : 4200)
struct BranchesSB {
    BranchesSBHeader header;
    Branch           branches[];
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
        const re::Buffer&                         vegBuf;
        uint32_t                                  maxVegCount;
        const re::StepDoubleBuffered<re::Buffer>& branchVectorBuf;
        const re::Buffer&                         branchRasterBuf;
    };

    VegStorage adoptSave(const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh);

private:
    /**
     * @brief Size of header is same as 2 branches
     */
    static constexpr int k_branchHeaderSize = sizeof(BranchesSBHeader) /
                                              sizeof(Branch);
    static_assert(k_branchHeaderSize * sizeof(Branch) == sizeof(BranchesSBHeader));

    struct VegDynamicsPC {
        glm::mat4 mvpMat;
        glm::vec2 worldTexSizeTi;
        float     timeSec = static_cast<float>(time(nullptr) & 0xFFFF);
    };
    VegDynamicsPC m_vegDynamicsPC;

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
    re::StepDoubleBuffered<re::DescriptorSet> m_descriptorSets{
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)},
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)}};
    re::Buffer                         m_vegBuf;
    re::StepDoubleBuffered<re::Buffer> m_branchVectorBuf;
    re::Buffer                         m_branchRasterBuf;
    re::Framebuffer                    m_framebuffer;
    glm::uvec2                         m_worldTexSizeTi{};
};

} // namespace rw
