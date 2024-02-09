﻿/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/utility/Math.hpp>

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

glm::uvec3 getAnalysisGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil(
            (viewSizeTi + glm::vec2(k_lightMaxRangeTi) * 2.0f) /
            k_analysisGroupSize / k_lightScale
        ),
        1u};
}

constexpr glm::vec2 k_calcGroupSize = glm::vec2{8.0f};
glm::uvec3          getShadowsCalculationGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil((viewSizeTi + k_lightScale * 2.0f) / k_calcGroupSize / k_lightScale),
        1u};
}

ShadowDrawer::ShadowDrawer(
    glm::vec2 viewSizePx, glm::ivec2 viewSizeTi, glm::uint maxNumberOfExternalLights
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
    , m_analyzeTilesPl(
          {.pipelineLayout = *m_analysisPll,
           .debugName      = "rw::ShadowDrawer::analyzeTiles"},
          {.comp = analyzeTiles_comp}
      )
    , m_addLightsPl(
          {.pipelineLayout = *m_analysisPll,
           .debugName      = "rw::ShadowDrawer::addLights"},
          {.comp = addDynamicLights_comp}
      )
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
          {.pipelineLayout = *m_calculationPll,
           .debugName      = "rw::ShadowDrawer::calculateShadows"},
          {.comp = calculateShadows_comp}
      )
    , m_shadowDrawingPll({}, {.vert = drawShadows_vert, .frag = drawColor_frag})
    , m_drawShadowsPl(
          re::PipelineGraphicsCreateInfo{
              .topology       = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout = *m_shadowDrawingPll,
              .debugName      = "rw::ShadowDrawer::drawShadows"},
          {.vert = drawShadows_vert, .frag = drawColor_frag}
      )
    , m_lightsBuf(re::BufferCreateInfo{
          .allocFlags  = eMapped | eHostAccessSequentialWrite,
          .sizeInBytes = maxNumberOfExternalLights * sizeof(ExternalLight),
          .usage       = vk::BufferUsageFlagBits::eStorageBuffer,
          .debugName   = "rw::ShadowDrawer::lights"})
    , m_(viewSizePx,
         viewSizeTi,
         m_analysisPll,
         m_calculationPll,
         m_shadowDrawingPll,
         m_blockLightAtlasTex,
         m_wallLightAtlasTex,
         m_lightsBuf) {
}

void ShadowDrawer::setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize) {
    m_.analysisPC.worldTexMask = worldTexSize - 1;
    m_.analysisDS.write(eCombinedImageSampler, 2u, 0u, worldTexture, eReadOnlyOptimal);
}

void ShadowDrawer::resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi) {
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

void ShadowDrawer::analyze(const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftTi) {
    m_.analysisPC.analysisOffsetTi = (botLeftTi - glm::ivec2(k_lightMaxRangeTi)) &
                                     ~k_lightScaleBits;
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeTilesPl);
    cmdBuf->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_analysisPll, 0u, *m_.analysisDS, {}
    );
    cmdBuf->pushConstants<AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
    cmdBuf->dispatch(
        m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z
    );
    m_.analysisPC.lightCount = 0;
}

void ShadowDrawer::addExternalLight(glm::ivec2 posPx, re::Color col) {
    ExternalLight light{posPx, col};
    std::memcpy(&m_lightsBuf[m_.analysisPC.lightCount], &light, sizeof(ExternalLight));
    m_.analysisPC.lightCount++;
}

void ShadowDrawer::calculate(const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftPx) {
    if (m_.analysisPC.lightCount > 0) { // If there are any dynamic lights
        // Wait for the analysis to be finished
        auto imageBarrier = re::imageMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            eGeneral,                                       // Old image layout
            eGeneral,                                       // New image layout
            m_.lightTex.image()
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        // Add dynamic lights
        m_.analysisPC.addLightOffsetPx =
            ((botLeftPx - k_lightMaxRangeTi * iTilePx) & k_unitMask) +
            k_halfUnitOffset;
        cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_addLightsPl);
        cmdBuf->pushConstants<AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
        cmdBuf->dispatch(re::ceilDiv(m_.analysisPC.lightCount, 8u), 1u, 1u);
    }

    { // Wait for the light and traslu texture to be written
        auto imageBarriers = std::to_array(
            {re::imageMemoryBarrier(
                 S::eComputeShader, // Src stage mask
                 A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
                 S::eComputeShader,      // Dst stage mask
                 A::eShaderSampledRead,  // Dst access mask
                 eGeneral,               // Old image layout
                 eShaderReadOnlyOptimal, // New image layout
                 m_.lightTex.image()
             ),
             re::imageMemoryBarrier(
                 S::eComputeShader, // Src stage mask
                 A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
                 S::eComputeShader,      // Dst stage mask
                 A::eShaderSampledRead,  // Dst access mask
                 eGeneral,               // Old image layout
                 eShaderReadOnlyOptimal, // New image layout
                 m_.transluTex.image()
             ),
             re::imageMemoryBarrier(
                 S::eVertexShader,       // Src stage mask
                 A::eShaderSampledRead,  // Src access mask
                 S::eComputeShader,      // Dst stage mask
                 A::eShaderStorageWrite, // Dst access mask
                 eShaderReadOnlyOptimal, // Old image layout
                 eGeneral,               // New image layout
                 m_.shadowsTex.image()
             )}
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    }

    // Calculate shadows
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_calculateShadowsPl);
    cmdBuf->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_calculationPll, 0u, *m_.calculationDS, {}
    );
    cmdBuf->dispatch(
        m_.calculationGroupCount.x,
        m_.calculationGroupCount.y,
        m_.calculationGroupCount.z
    );

    { // Reverse layout transitions
        auto imageBarriers = std::to_array(
            {re::imageMemoryBarrier(
                 S::eComputeShader,     // Src stage mask
                 A::eShaderSampledRead, // Src access mask
                 S::eComputeShader,     // Dst stage mask
                 A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
                 eShaderReadOnlyOptimal, // Old image layout
                 eGeneral,               // New image layout
                 m_.lightTex.image()
             ),
             re::imageMemoryBarrier(
                 S::eComputeShader,     // Src stage mask
                 A::eShaderSampledRead, // Src access mask
                 S::eComputeShader,     // Dst stage mask
                 A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
                 eShaderReadOnlyOptimal, // Old image layout
                 eGeneral,               // New image layout
                 m_.transluTex.image()
             ),
             re::imageMemoryBarrier(
                 S::eComputeShader,      // Src stage mask
                 A::eShaderStorageWrite, // Src access mask
                 S::eVertexShader,       // Dst stage mask
                 A::eShaderSampledRead,  // Dst access mask
                 eGeneral,               // Old image layout
                 eShaderReadOnlyOptimal, // New image layout
                 m_.shadowsTex.image()
             )}
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    }
}

void ShadowDrawer::draw(
    const re::CommandBuffer& cmdBuf, glm::vec2 botLeftPx, glm::uvec2 viewSizeTi
) {
    m_.shadowDrawingPC.botLeftPxModTilePx = glm::mod(botLeftPx, TilePx);
    m_.shadowDrawingPC.readOffsetTi       = glm::ivec2(pxToTi(botLeftPx)) &
                                      k_lightScaleBits;
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawShadowsPl);
    cmdBuf->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_shadowDrawingPll, 0u, *m_.shadowDrawingDS, {}
    );
    cmdBuf->pushConstants<ShadowDrawingPC>(
        *m_shadowDrawingPll, eVertex, 0u, m_.shadowDrawingPC
    );
    cmdBuf->draw(4u, viewSizeTi.x * viewSizeTi.y, 0u, 0u);
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(
    glm::vec2                 viewSizePx,
    glm::ivec2                viewSizeTi,
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
          .magFilter = vk::Filter::eLinear,
          .debugName = "rw::ShadowDrawer::light"})
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
          .magFilter     = vk::Filter::eLinear,
          .debugName     = "rw::ShadowDrawer::translu"})
    , shadowsTex(re::TextureCreateInfo{
          .extent = {glm::vec2{calculationGroupCount} * k_calcGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .magFilter = vk::Filter::eLinear,
          .debugName = "rw::ShadowDrawer::shadows"})
    , shadowDrawingPC(
          {.viewMat    = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y),
           .viewSizeTi = viewSizeTi}
      )
    , analysisDS(re::DescriptorSetCreateInfo{
          .layout    = analysisPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::analysis"})
    , calculationDS(re::DescriptorSetCreateInfo{
          .layout    = calculationPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::calculation"})
    , shadowDrawingDS(re::DescriptorSetCreateInfo{
          .layout    = shadowDrawingPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::shadowDrawing"}) {
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
    analysisDS.write(eStorageBuffer, 5u, 0u, lightsBuf, 0ull, vk::WholeSize);

    // Calculation descriptor set
    calculationDS.write(eCombinedImageSampler, 0u, 0u, lightTex, eReadOnlyOptimal);
    calculationDS.write(eCombinedImageSampler, 1u, 0u, transluTex, eReadOnlyOptimal);
    calculationDS.write(eStorageImage, 2u, 0u, shadowsTex, eGeneral);

    // Shadow drawing descriptor set
    shadowDrawingDS.write(eCombinedImageSampler, 0u, 0u, shadowsTex, eReadOnlyOptimal);
}

} // namespace rw
