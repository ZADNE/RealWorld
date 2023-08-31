/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/light.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/ShadowDrawer.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using enum vk::MemoryPropertyFlagBits;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;
using enum vma::AllocationCreateFlagBits;

namespace rw {

constexpr int       k_unitMask          = ~(k_iLightScale * iTilePx.x - 1);
constexpr int       k_halfUnitOffset    = iTilePx.x * k_iLightScale / 2;
constexpr glm::vec2 k_analysisGroupSize = glm::vec2{8.0f};

glm::uvec3 getAnalysisGroupCount(const glm::vec2& viewSizeTi) {
    return {
        glm::ceil(
            (viewSizeTi + glm::vec2(k_lightMaxRangeTi) * 2.0f) /
            k_analysisGroupSize / k_lightScale
        ),
        1u};
}

constexpr glm::vec2 k_calcGroupSize = glm::vec2{8.0f};
glm::uvec3 getShadowsCalculationGroupCount(const glm::vec2& viewSizeTi) {
    return {
        glm::ceil((viewSizeTi + k_lightScale * 2.0f) / k_calcGroupSize / k_lightScale),
        1u};
}

ShadowDrawer::ShadowDrawer(
    const glm::vec2&  viewSizePx,
    const glm::ivec2& viewSizeTi,
    glm::uint         maxNumberOfExternalLights
)
    : m_analysisPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eStorageImage, 1u, eCompute},         // lightImage
                  {1u, eStorageImage, 1u, eCompute},         // transluImage
                  {2u, eCombinedImageSampler, 1u, eCompute}, // worldSampler
                  {3u, eCombinedImageSampler, 1u, eCompute}, // blockLightAtlas
                  {4u, eCombinedImageSampler, 1u, eCompute}, // wallLightAtlas
                  {5u, eStorageBuffer, 1u, eCompute},        // DynamicLightsSB
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(AnalysisPC)}}}
      )
    , m_analyzeTilesPl({.pipelineLayout = *m_analysisPll}, {.comp = analyzeTiles_comp})
    , m_addLightsPl({.pipelineLayout = *m_analysisPll}, {.comp = addDynamicLights_comp})
    , m_calculationPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eCompute}, // lightSampler
                  {1u, eCombinedImageSampler, 1u, eCompute}, // transluSampler
                  {2u, eStorageImage, 1u, eCompute},         // shadowsImage
              }}}
      )
    , m_calculateShadowsPl(
          {.pipelineLayout = *m_calculationPll}, {.comp = calculateShadows_comp}
      )
    , m_shadowDrawingPll({}, {.vert = drawShadows_vert, .frag = drawColor_frag})
    , m_drawShadowsPl(
          re::PipelineGraphicsCreateInfo{
              .pipelineLayout = *m_shadowDrawingPll,
              .topology       = vk::PrimitiveTopology::eTriangleStrip},
          {.vert = drawShadows_vert, .frag = drawColor_frag}
      )
    , m_lightsBuf(re::BufferCreateInfo{
          .allocFlags  = eMapped | eHostAccessSequentialWrite,
          .sizeInBytes = maxNumberOfExternalLights * sizeof(ExternalLight),
          .usage       = vk::BufferUsageFlagBits::eStorageBuffer})
    , m_(viewSizePx,
         viewSizeTi,
         m_analysisPll,
         m_calculationPll,
         m_shadowDrawingPll,
         m_blockLightAtlasTex,
         m_wallLightAtlasTex,
         m_lightsBuf) {
}

void ShadowDrawer::setTarget(
    const re::Texture& worldTexture, const glm::ivec2& worldTexSize
) {
    m_.analysisPC.worldTexMask = worldTexSize - 1;
    m_.analysisDS.write(eCombinedImageSampler, 2u, 0u, worldTexture, eReadOnlyOptimal);
}

void ShadowDrawer::resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi) {
    m_ = ViewSizeDependent{
        viewSizePx,
        viewSizeTi,
        m_analysisPll,
        m_calculationPll,
        m_shadowDrawingPll,
        m_blockLightAtlasTex,
        m_wallLightAtlasTex,
        m_lightsBuf};
}

void ShadowDrawer::analyze(
    const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi
) {
    m_.analysisPC.analysisOffsetTi = (botLeftTi - glm::ivec2(k_lightMaxRangeTi)) &
                                     ~k_lightScaleBits;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeTilesPl);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_analysisPll, 0u, *m_.analysisDS, {}
    );
    commandBuffer.pushConstants<AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
    commandBuffer.dispatch(
        m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z
    );
    m_.analysisPC.lightCount = 0;
}

void ShadowDrawer::addExternalLight(const glm::ivec2& posPx, re::Color col) {
    ExternalLight light{posPx, col};
    std::memcpy(&m_lightsBuf[m_.analysisPC.lightCount], &light, sizeof(ExternalLight));
    m_.analysisPC.lightCount++;
}

void ShadowDrawer::calculate(
    const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftPx
) {
    if (m_.analysisPC.lightCount > 0) { // If there are any dynamic lights
        // Wait for the analysis to be finished
        auto imageBarrier = imageMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            eGeneral,                                       // Old image layout
            eGeneral,                                       // New image layout
            m_.lightTex.image()
        );
        commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        // Add dynamic lights
        m_.analysisPC.addLightOffsetPx =
            ((botLeftPx - k_lightMaxRangeTi * iTilePx) & k_unitMask) +
            k_halfUnitOffset;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_addLightsPl);
        commandBuffer.pushConstants<AnalysisPC>(
            *m_analysisPll, eCompute, 0u, m_.analysisPC
        );
        commandBuffer.dispatch(1u + (m_.analysisPC.lightCount - 1u) / 8u, 1u, 1u);
    }

    // Wait for the light and traslu texture to be written
    vk::ImageMemoryBarrier2 imageBarriers[] = {
        imageMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            eGeneral,                                       // Old image layout
            eShaderReadOnlyOptimal,                         // New image layout
            m_.lightTex.image()
        ),
        imageMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            eGeneral,                                       // Old image layout
            eShaderReadOnlyOptimal,                         // New image layout
            m_.transluTex.image()
        ),
        imageMemoryBarrier(
            S::eVertexShader,       // Src stage mask
            A::eShaderSampledRead,  // Src access mask
            S::eComputeShader,      // Dst stage mask
            A::eShaderStorageWrite, // Dst access mask
            eShaderReadOnlyOptimal, // Old image layout
            eGeneral,               // New image layout
            m_.shadowsTex.image()
        )};

    // Calculate shadows
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_calculateShadowsPl);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_calculationPll, 0u, *m_.calculationDS, {}
    );
    commandBuffer.dispatch(
        m_.calculationGroupCount.x,
        m_.calculationGroupCount.y,
        m_.calculationGroupCount.z
    );

    // Reverse layout transitions
    std::swap(imageBarriers[0].oldLayout, imageBarriers[0].newLayout);
    std::swap(imageBarriers[1].oldLayout, imageBarriers[1].newLayout);
    std::swap(imageBarriers[2].oldLayout, imageBarriers[2].newLayout);
    std::swap(imageBarriers[2].srcStageMask, imageBarriers[2].dstStageMask);
    std::swap(imageBarriers[2].srcStageMask, imageBarriers[2].dstStageMask);
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
}

void ShadowDrawer::draw(
    const vk::CommandBuffer& commandBuffer,
    const glm::vec2&         botLeftPx,
    const glm::uvec2&        viewSizeTi
) {
    m_.shadowDrawingPC.botLeftPxModTilePx = glm::mod(botLeftPx, TilePx);
    m_.shadowDrawingPC.readOffsetTi       = glm::ivec2(pxToTi(botLeftPx)) &
                                      k_lightScaleBits;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawShadowsPl);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_shadowDrawingPll, 0u, *m_.shadowDrawingDS, {}
    );
    commandBuffer.pushConstants<ShadowDrawingPC>(
        *m_shadowDrawingPll, eVertex, 0u, m_.shadowDrawingPC
    );
    commandBuffer.draw(4u, viewSizeTi.x * viewSizeTi.y, 0u, 0u);
}

vk::ImageMemoryBarrier2 ShadowDrawer::imageMemoryBarrier(
    vk::PipelineStageFlags2 srcStageMask,
    vk::AccessFlags2        srcAccessMask,
    vk::PipelineStageFlags2 dstStageMask,
    vk::AccessFlags2        dstAccessMask,
    vk::ImageLayout         oldLayout,
    vk::ImageLayout         newLayout,
    vk::Image               image
) {
    return vk::ImageMemoryBarrier2{
        srcStageMask,
        srcAccessMask,
        dstStageMask,
        dstAccessMask,
        oldLayout,
        newLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // No ownership transition
        image,
        vk::ImageSubresourceRange{eColor, 0u, 1u, 0u, 1u} // Whole image
    };
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(
    const glm::vec2&          viewSizePx,
    const glm::ivec2&         viewSizeTi,
    const re::PipelineLayout& analysisPll,
    const re::PipelineLayout& calculationPll,
    const re::PipelineLayout& shadowDrawingPll,
    const re::Texture&        blockLightAtlasTex,
    const re::Texture&        wallLightAtlasTex,
    const re::Buffer&         lightsBuf
)
    : analysisGroupCount(getAnalysisGroupCount(viewSizeTi))
    , calculationGroupCount(getShadowsCalculationGroupCount(viewSizeTi))
    , lightTex(re::TextureCreateInfo{
          .flags  = vk::ImageCreateFlagBits::eMutableFormat,
          .format = vk::Format::eR8G8B8A8Unorm,
          .extent = {glm::vec2{analysisGroupCount} * k_analysisGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .initialLayout = eGeneral,
          .pNext =
              [] {
                  constexpr static auto formats = std::to_array(
                      {vk::Format::eR8G8B8A8Unorm, vk::Format::eR32Uint}
                  );
                  constexpr static vk::ImageFormatListCreateInfo formatList{
                      formats.size(), formats.data()};
                  return &formatList;
              }(),
          .magFilter = vk::Filter::eLinear})
    , lightTexR32ImageView(vk::ImageViewCreateInfo{
          {},
          lightTex.image(),
          vk::ImageViewType::e2D,
          vk::Format::eR32Uint,
          vk::ComponentMapping{},
          vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}})
    , transluTex(re::TextureCreateInfo{
          .format = vk::Format::eR8Unorm,
          .extent = {glm::vec2{analysisGroupCount} * k_analysisGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .initialLayout = eGeneral,
          .magFilter     = vk::Filter::eLinear})
    , shadowsTex(re::TextureCreateInfo{
          .extent = {glm::vec2{calculationGroupCount} * k_calcGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .magFilter = vk::Filter::eLinear})
    , shadowDrawingPC(
          {.viewMat    = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y),
           .viewSizeTi = viewSizeTi}
      )
    , analysisDS(analysisPll.descriptorSetLayout(0))
    , calculationDS(calculationPll.descriptorSetLayout(0))
    , shadowDrawingDS(shadowDrawingPll.descriptorSetLayout(0)) {
    using enum vk::DescriptorType;
    // Analysis descriptor set
    analysisDS.write(
        eStorageImage,
        0u,
        0u,
        vk::DescriptorImageInfo{lightTex.sampler(), *lightTexR32ImageView, eGeneral}
    );
    analysisDS.write(eStorageImage, 1u, 0u, transluTex, eGeneral);
    analysisDS.write(eCombinedImageSampler, 3u, 0u, blockLightAtlasTex, eReadOnlyOptimal);
    analysisDS.write(eCombinedImageSampler, 4u, 0u, wallLightAtlasTex, eReadOnlyOptimal);
    analysisDS.write(eStorageBuffer, 5u, 0u, lightsBuf, 0ull, VK_WHOLE_SIZE);
    // Calculation descriptor set
    calculationDS.write(eCombinedImageSampler, 0u, 0u, lightTex, eReadOnlyOptimal);
    calculationDS.write(eCombinedImageSampler, 1u, 0u, transluTex, eReadOnlyOptimal);
    calculationDS.write(eStorageImage, 2u, 0u, shadowsTex, eGeneral);
    // Shadow drawing descriptor set
    shadowDrawingDS.write(eCombinedImageSampler, 0u, 0u, shadowsTex, eReadOnlyOptimal);
}

} // namespace rw
