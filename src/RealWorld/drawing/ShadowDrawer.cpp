/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/ShadowDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/utility/Math.hpp>

#include <RealWorld/constants/Tile.hpp>
#include <RealWorld/drawing/shaders/AllShaders.gen.hpp>
#include <RealWorld/drawing/shaders/analysisPll.glsl.gen.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using enum vk::MemoryPropertyFlagBits;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;
using enum vma::AllocationCreateFlagBits;

namespace rw {

using namespace glsl;

static_assert(1 << k_lightMinCellTiBitShift == k_lightMinCellTi);
static_assert(1 << k_lightMaxCellTiBitShift == k_lightMaxCellTi);
static_assert(
    static_cast<int>(k_analysisGroupSize) >= k_lightMaxCellTi / k_lightMinCellTi,
    "Analysis group must be big enough to calculate the highest mip"
);
constexpr float k_lightSweepBaseUvOffset =
    float(k_lightMaxRangeTi >> k_lightMinCellTiBitShift) - 0.5f;

glm::uvec3 calcAnalysisGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil(
            (viewSizeTi + static_cast<float>(k_lightMaxCellTi) +
             glm::vec2(k_lightMaxRangeTi) * 2.0f) /
            (k_analysisGroupSize * k_lightMinCellTi)
        ),
        1u
    };
}

glm::uvec3 calcLightSweepGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil(
            (viewSizeTi + k_lightMinCellTi * 2.0f) / k_lightSweepGroupSize /
            static_cast<float>(k_lightMinCellTi)
        ),
        1u
    };
}

ShadowDrawer::ShadowDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
    glm::ivec2 viewSizeTi, glm::uint maxNumberOfExternalLights,
    glsl::WorldDrawingPC& pc
)
    : m_pc(pc)
    , m_analysisPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {k_lightXluImageBinding, eStorageImage, k_lightCellSizeCount,
                   eCompute},
                  {k_worldSamplerBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_blockLightAtlasBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_wallLightAtlasBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_externalLightsBinding, eStorageBuffer, 1u, eCompute},
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(glsl::AnalysisPC)}}
          }
      )
    , m_analyzeTilesPl(
          {.pipelineLayout = *m_analysisPll,
           .debugName      = "rw::ShadowDrawer::analyzeTiles"},
          {.comp = glsl::analyzeTiles_comp}
      )
    , m_addExternalLightsPl(
          {.pipelineLayout = *m_analysisPll,
           .debugName      = "rw::ShadowDrawer::addLights"},
          {.comp = glsl::addExternalLights_comp}
      )
    , m_lightSweepPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eCompute}, // lightXluSampler
                  {1u, eStorageImage, 1u, eCompute},         // shadowsImage
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(glsl::LightSweepPC)}}
          }
      )
    , m_sweepLightPl(
          {.pipelineLayout = *m_lightSweepPll,
           .debugName      = "rw::ShadowDrawer::calculateShadows"},
          {.comp = glsl::sweepLight_comp}
      )
    , m_shadowDrawingPll(
          {}, {.vert = glsl::drawFullscreen_vert, .frag = glsl::drawShadows_frag}
      )
    , m_drawShadowsPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout    = *m_shadowDrawingPll,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::ShadowDrawer::drawShadows"
          },
          {.vert = glsl::drawFullscreen_vert, .frag = glsl::drawShadows_frag}
      )
    , m_lightsBuf(
          re::BufferCreateInfo{
              .allocFlags = eMapped | eHostAccessSequentialWrite,
              .sizeInBytes = maxNumberOfExternalLights * sizeof(glsl::ExternalLight),
              .usage     = vk::BufferUsageFlagBits::eStorageBuffer,
              .debugName = "rw::ShadowDrawer::lights"
          }
      )
    , m_(viewSizePx, viewSizeTi, m_analysisPll, m_lightSweepPll, m_shadowDrawingPll,
         m_blockLightAtlasTex, m_wallLightAtlasTex, m_lightsBuf) {
}

void ShadowDrawer::setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize) {
    m_.analysisPC.worldTexMask = worldTexSize - 1;
    m_.calcInputsDS.write(
        eCombinedImageSampler, k_worldSamplerBinding, 0u, worldTexture,
        eShaderReadOnlyOptimal
    );
}

void ShadowDrawer::resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi) {
    m_ = ViewSizeDependent{viewSizePx,          viewSizeTi,
                           m_analysisPll,       m_lightSweepPll,
                           m_shadowDrawingPll,  m_blockLightAtlasTex,
                           m_wallLightAtlasTex, m_lightsBuf};
}

void ShadowDrawer::analyze(
    const re::CommandBuffer& cb, glm::ivec2 botLeftTi, float timeD
) {
    // Align analysis to a multiple of max cell size
    m_.analysisPC.timeD = timeD;
    m_.analysisPC.analysisOffsetTi = (botLeftTi - glm::ivec2(k_lightMaxRangeTi)) &
                                     ~k_lightMaxCellTiMask;
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeTilesPl);
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_analysisPll, 0u, *m_.calcInputsDS, {}
    );
    cb->pushConstants<glsl::AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
    cb->dispatch(
        m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z
    );
    m_.analysisPC.lightCount = 0;
}

void ShadowDrawer::addExternalLight(glm::ivec2 posPx, glm::vec3 light) {
    glsl::ExternalLight extLight{posPx, light.r, light.g, light.b, 0.0f};
    std::memcpy(&m_lightsBuf[m_.analysisPC.lightCount], &extLight, sizeof(extLight));
    m_.analysisPC.lightCount++;
}

void ShadowDrawer::calculate(const re::CommandBuffer& cb, glm::ivec2 botLeftPx) {
    if (m_.analysisPC.lightCount > 0) { // If there are any external lights
        // Wait for the analysis to be finished
        auto imageBarrier = re::imageMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            eGeneral,                                       // Old image layout
            eGeneral,                                       // New image layout
            m_.lightXluTex.image()
        );
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        // Add dynamic lights
        m_.analysisPC.addLightOffsetPx = ((botLeftPx - tiToPx(k_lightMaxRangeTi)) &
                                          (tiToPx(k_lightMinCellTi) - 1)) +
                                         tiToPx(k_lightMinCellTi) / 2;
        /* m_.analysisPC.addLightOffsetPx =
            ((botLeftPx - tiToPx(k_lightMaxRangeTi)) & k_unitMask) +
            k_halfUnitOffset;*/
        cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_addExternalLightsPl);
        cb->pushConstants<glsl::AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
        cb->dispatch(
            re::ceilDiv(m_.analysisPC.lightCount, glsl::k_addExternalLightsGroupSize),
            1u, 1u
        );
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
                 m_.lightXluTex.image()
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
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    }

    // Calculate shadows
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_sweepLightPl);
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_lightSweepPll, 0u, *m_.calculationDS, {}
    );
    // Align in min cell sizes within max cell size
    constexpr auto k_minInMax = (k_lightMaxCellTi / k_lightMinCellTi) - 1;
    m_.lightSweepPC.uvOffset =
        glm::vec2{(pxToTi(botLeftPx) >> k_lightMinCellTiBitShift) & k_minInMax} +
        k_lightSweepBaseUvOffset;
    cb->pushConstants<glsl::LightSweepPC>(*m_lightSweepPll, eCompute, 0u, m_.lightSweepPC);
    cb->dispatch(
        m_.lightSweepGroupCount.x, m_.lightSweepGroupCount.y,
        m_.lightSweepGroupCount.z
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
                 m_.lightXluTex.image()
             ),
             re::imageMemoryBarrier(
                 S::eComputeShader,      // Src stage mask
                 A::eShaderStorageWrite, // Src access mask
                 S::eFragmentShader,     // Dst stage mask
                 A::eShaderSampledRead,  // Dst access mask
                 eGeneral,               // Old image layout
                 eShaderReadOnlyOptimal, // New image layout
                 m_.shadowsTex.image()
             )}
        );
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    }
}

void ShadowDrawer::draw(const re::CommandBuffer& cb, glm::vec2 botLeftPx) {
    constexpr float k_cellTi = k_lightMinCellTi;

    m_pc.uvRectSize = m_.viewSizePx * m_.shadowAreaPxInv;
    m_pc.uvRectOffset = (glm::mod(botLeftPx, tiToPx(k_cellTi)) + tiToPx(k_cellTi)) *
                        m_.shadowAreaPxInv;
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawShadowsPl);
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_shadowDrawingPll, 0u,
        *m_.shadowDrawingDS, {}
    );
    cb->pushConstants<glsl::WorldDrawingPC>(*m_shadowDrawingPll, eVertex, 0u, m_pc);
    cb->draw(3u, 1u, 0u, 0u);
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(
    glm::vec2 viewSizePx, glm::ivec2 viewSizeTi,
    const re::PipelineLayout& analysisPll, const re::PipelineLayout& lightSweepPll,
    const re::PipelineLayout& shadowDrawingPll,
    const re::Texture& blockLightAtlasTex, const re::Texture& wallLightAtlasTex,
    const re::Buffer& lightsBuf
)
    : viewSizePx(viewSizePx)
    , analysisGroupCount(calcAnalysisGroupCount(viewSizeTi))
    , lightSweepGroupCount(calcLightSweepGroupCount(viewSizeTi))
    , lightXluTex(
          re::TextureCreateInfo{
              .flags  = vk::ImageCreateFlagBits::eMutableFormat,
              .format = vk::Format::eR16G16Sfloat,
              .extent = {glm::vec2{analysisGroupCount} * k_analysisGroupSize, 1u},
              .mipLevels = k_lightCellSizeCount,
              .layers    = 2,
              .usage     = vk::ImageUsageFlagBits::eSampled |
                       vk::ImageUsageFlagBits::eStorage,
              .initialLayout = eGeneral,
              .pNext =
                  [] {
                      constexpr static auto k_formats = std::to_array(
                          {vk::Format::eR16G16Sfloat, vk::Format::eR32Uint}
                      );
                      constexpr static vk::ImageFormatListCreateInfo k_formatList{
                          k_formats.size(), k_formats.data()
                      };
                      return &k_formatList;
                  }(),
              .magFilter = vk::Filter::eLinear,
              .minFilter = vk::Filter::eLinear,
              .debugName = "rw::ShadowDrawer::lightXluTex"
          }
      )
    , shadowsTex(
          re::TextureCreateInfo{
              .extent = {glm::vec2{lightSweepGroupCount} * k_lightSweepGroupSize, 1u},
              .usage = vk::ImageUsageFlagBits::eSampled |
                       vk::ImageUsageFlagBits::eStorage,
              .magFilter = vk::Filter::eLinear,
              .debugName = "rw::ShadowDrawer::shadows"
          }
      )
    , lightSweepPC{.uvScale = 1.0f / (glm::vec2{analysisGroupCount} * k_analysisGroupSize)}
    , calcInputsDS(
          re::DescriptorSetCreateInfo{
              .layout    = analysisPll.descriptorSetLayout(0),
              .debugName = "rw::ShadowDrawer::analysis"
          }
      )
    , calculationDS(
          re::DescriptorSetCreateInfo{
              .layout    = lightSweepPll.descriptorSetLayout(0),
              .debugName = "rw::ShadowDrawer::calculation"
          }
      )
    , shadowDrawingDS(
          re::DescriptorSetCreateInfo{
              .layout    = shadowDrawingPll.descriptorSetLayout(0),
              .debugName = "rw::ShadowDrawer::shadowDrawing"
          }
      )
    , shadowAreaPxInv(
          glm::vec2{1.0f} /
          tiToPx(
              glm::vec2{lightSweepGroupCount} * k_lightSweepGroupSize *
              static_cast<float>(k_lightMinCellTi)
          )
      ) {
    // Initialize image views
    vk::ImageViewCreateInfo imageViewCreateinfo{
        {},
        lightXluTex.image(),
        vk::ImageViewType::e2DArray,
        vk::Format::eR32Uint,
        vk::ComponentMapping{},
        vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor, ~0u, 1, 0, vk::RemainingArrayLayers
        }
    };
    for (int i = 0; i < k_lightCellSizeCount; ++i) {
        imageViewCreateinfo.subresourceRange.baseMipLevel = i;
        lightXluImgViews32ui[i] = re::ImageView{imageViewCreateinfo};
    }

    using enum vk::DescriptorType;

    // Shadow inputs descriptor set
    for (int i = 0; i < k_lightCellSizeCount; ++i) {
        calcInputsDS.write(
            eStorageImage, k_lightXluImageBinding, i,
            vk::DescriptorImageInfo{nullptr, *lightXluImgViews32ui[i], eGeneral}
        );
    }
    calcInputsDS.write(
        eCombinedImageSampler, k_blockLightAtlasBinding, 0u, blockLightAtlasTex,
        eShaderReadOnlyOptimal
    );
    calcInputsDS.write(
        eCombinedImageSampler, k_wallLightAtlasBinding, 0u, wallLightAtlasTex,
        eShaderReadOnlyOptimal
    );
    calcInputsDS.write(eStorageBuffer, k_externalLightsBinding, 0u, lightsBuf);

    // Calculation descriptor set
    calculationDS.write(
        eCombinedImageSampler, 0u, 0u, lightXluTex, eShaderReadOnlyOptimal
    );
    calculationDS.write(eStorageImage, 1u, 0u, shadowsTex, eGeneral);

    // Shadow drawing descriptor set
    shadowDrawingDS.write(
        eCombinedImageSampler, 0u, 0u, shadowsTex, eShaderReadOnlyOptimal
    );
}

} // namespace rw
