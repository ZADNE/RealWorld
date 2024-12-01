/*!
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

constexpr int k_unitMask                = ~(k_iLightScale * iTilePx.x - 1);
constexpr int k_halfUnitOffset          = iTilePx.x * k_iLightScale / 2;
constexpr glm::vec2 k_analysisGroupSize = glm::vec2{8.0f};

constexpr glm::uint k_lightImageBinding      = 0;
constexpr glm::uint k_transluImageBinding    = 1;
constexpr glm::uint k_worldSamplerBinding    = 2;
constexpr glm::uint k_blockLightAtlasBinding = 3;
constexpr glm::uint k_wallLightAtlasBinding  = 4;
constexpr glm::uint k_dynamicLightsBinding   = 5;

glm::uvec3 getAnalysisGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil(
            (viewSizeTi + glm::vec2(k_lightMaxRangeTi) * 2.0f) /
            k_analysisGroupSize / k_lightScale
        ),
        1u
    };
}

constexpr glm::vec2 k_calcGroupSize = glm::vec2{8.0f};
glm::uvec3 getShadowsCalculationGroupCount(glm::vec2 viewSizeTi) {
    return {
        glm::ceil((viewSizeTi + k_lightScale * 2.0f) / k_calcGroupSize / k_lightScale), 1u
    };
}

ShadowDrawer::ShadowDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
    glm::ivec2 viewSizeTi, glm::uint maxNumberOfExternalLights,
    glsl::WorldDrawingPC& pc
)
    : m_pc(pc)
    , m_calcInputsPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {k_lightImageBinding, eStorageImage, 1u, eCompute},
                  {k_transluImageBinding, eStorageImage, 1u, eCompute},
                  {k_worldSamplerBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_blockLightAtlasBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_wallLightAtlasBinding, eCombinedImageSampler, 1u, eCompute},
                  {k_dynamicLightsBinding, eStorageBuffer, 1u, eCompute},
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(glsl::AnalysisPC)}}
          }
      )
    , m_analyzeTilesPl(
          {.pipelineLayout = *m_calcInputsPll,
           .debugName      = "rw::ShadowDrawer::analyzeTiles"},
          {.comp = glsl::analyzeTiles_comp}
      )
    , m_addLightsPl(
          {.pipelineLayout = *m_calcInputsPll,
           .debugName      = "rw::ShadowDrawer::addLights"},
          {.comp = glsl::addDynamicLights_comp}
      )
    , m_calculationPll(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eCompute}, // lightSampler
                  {1u, eCombinedImageSampler, 1u, eCompute}, // transluSampler
                  {2u, eStorageImage, 1u, eCompute},         // shadowsImage
              }}
          }
      )
    , m_calculateShadowsPl(
          {.pipelineLayout = *m_calculationPll,
           .debugName      = "rw::ShadowDrawer::calculateShadows"},
          {.comp = glsl::calculateShadows_comp}
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
    , m_lightsBuf(re::BufferCreateInfo{
          .allocFlags  = eMapped | eHostAccessSequentialWrite,
          .sizeInBytes = maxNumberOfExternalLights * sizeof(glsl::DynamicLight),
          .usage       = vk::BufferUsageFlagBits::eStorageBuffer,
          .debugName   = "rw::ShadowDrawer::lights"
      })
    , m_(viewSizePx, viewSizeTi, m_calcInputsPll, m_calculationPll,
         m_shadowDrawingPll, m_blockLightAtlasTex, m_wallLightAtlasTex,
         m_lightsBuf) {
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
                           m_calcInputsPll,     m_calculationPll,
                           m_shadowDrawingPll,  m_blockLightAtlasTex,
                           m_wallLightAtlasTex, m_lightsBuf};
}

void ShadowDrawer::analyze(
    const re::CommandBuffer& cb, glm::ivec2 botLeftTi, const glm::vec4& skyLight
) {
    m_.analysisPC.skyLight = skyLight;
    m_.analysisPC.analysisOffsetTi = (botLeftTi - glm::ivec2(k_lightMaxRangeTi)) &
                                     ~k_lightScaleBits;
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeTilesPl);
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_calcInputsPll, 0u, *m_.calcInputsDS, {}
    );
    cb->pushConstants<glsl::AnalysisPC>(*m_calcInputsPll, eCompute, 0u, m_.analysisPC);
    cb->dispatch(
        m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z
    );
    m_.analysisPC.lightCount = 0;
}

void ShadowDrawer::addExternalLight(glm::ivec2 posPx, re::Color col) {
    glsl::DynamicLight light{posPx, std::bit_cast<glm::uint>(col), {}};
    std::memcpy(&m_lightsBuf->lights[m_.analysisPC.lightCount], &light, sizeof(light));
    m_.analysisPC.lightCount++;
}

void ShadowDrawer::calculate(const re::CommandBuffer& cb, glm::ivec2 botLeftPx) {
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
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        // Add dynamic lights
        m_.analysisPC.addLightOffsetPx =
            ((botLeftPx - k_lightMaxRangeTi * iTilePx) & k_unitMask) +
            k_halfUnitOffset;
        cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_addLightsPl);
        cb->pushConstants<glsl::AnalysisPC>(*m_calcInputsPll, eCompute, 0u, m_.analysisPC);
        cb->dispatch(re::ceilDiv(m_.analysisPC.lightCount, 8u), 1u, 1u);
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
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    }

    // Calculate shadows
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_calculateShadowsPl);
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_calculationPll, 0u,
        *m_.calculationDS, {}
    );
    cb->dispatch(
        m_.calculationGroupCount.x, m_.calculationGroupCount.y,
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
    m_pc.uvRectSize   = m_.viewSizePx * m_.shadowAreaPxInv;
    m_pc.uvRectOffset = (glm::mod(botLeftPx, TilePx * k_lightScale) +
                         TilePx * static_cast<float>(k_iLightScale)) *
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
    const re::PipelineLayout& shadowInputsPll,
    const re::PipelineLayout& calculationPll,
    const re::PipelineLayout& shadowDrawingPll,
    const re::Texture& blockLightAtlasTex, const re::Texture& wallLightAtlasTex,
    const re::Buffer& lightsBuf
)
    : viewSizePx(viewSizePx)
    , analysisGroupCount(getAnalysisGroupCount(viewSizeTi))
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
                      formats.size(), formats.data()
                  };
                  return &formatList;
              }(),
          .magFilter = vk::Filter::eLinear,
          .debugName = "rw::ShadowDrawer::light"
      })
    , lightTexR32ImageView(vk::ImageViewCreateInfo{
          {},
          lightTex.image(),
          vk::ImageViewType::e2D,
          vk::Format::eR32Uint,
          vk::ComponentMapping{},
          vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
      })
    , transluTex(re::TextureCreateInfo{
          .format = vk::Format::eR8Unorm,
          .extent = {glm::vec2{analysisGroupCount} * k_analysisGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .initialLayout = eGeneral,
          .magFilter     = vk::Filter::eLinear,
          .debugName     = "rw::ShadowDrawer::translu"
      })
    , shadowsTex(re::TextureCreateInfo{
          .extent = {glm::vec2{calculationGroupCount} * k_calcGroupSize, 1u},
          .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
          .magFilter = vk::Filter::eLinear,
          .debugName = "rw::ShadowDrawer::shadows"
      })
    , calcInputsDS(re::DescriptorSetCreateInfo{
          .layout    = shadowInputsPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::analysis"
      })
    , calculationDS(re::DescriptorSetCreateInfo{
          .layout    = calculationPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::calculation"
      })
    , shadowDrawingDS(re::DescriptorSetCreateInfo{
          .layout    = shadowDrawingPll.descriptorSetLayout(0),
          .debugName = "rw::ShadowDrawer::shadowDrawing"
      })
    , shadowAreaPxInv(
          glm::vec2{1.0f} /
          tiToPx(glm::vec2{calculationGroupCount} * k_calcGroupSize * k_lightScale)
      ) {
    using enum vk::DescriptorType;

    // Shadow inputs descriptor set
    calcInputsDS.write(
        eStorageImage, k_lightImageBinding, 0u,
        vk::DescriptorImageInfo{lightTex.sampler(), *lightTexR32ImageView, eGeneral}
    );
    calcInputsDS.write(eStorageImage, k_transluImageBinding, 0u, transluTex, eGeneral);
    calcInputsDS.write(
        eCombinedImageSampler, k_blockLightAtlasBinding, 0u, blockLightAtlasTex,
        eShaderReadOnlyOptimal
    );
    calcInputsDS.write(
        eCombinedImageSampler, k_wallLightAtlasBinding, 0u, wallLightAtlasTex,
        eShaderReadOnlyOptimal
    );
    calcInputsDS.write(eStorageBuffer, k_dynamicLightsBinding, 0u, lightsBuf);

    // Calculation descriptor set
    calculationDS.write(eCombinedImageSampler, 0u, 0u, lightTex, eShaderReadOnlyOptimal);
    calculationDS.write(eCombinedImageSampler, 1u, 0u, transluTex, eShaderReadOnlyOptimal);
    calculationDS.write(eStorageImage, 2u, 0u, shadowsTex, eGeneral);

    // Shadow drawing descriptor set
    shadowDrawingDS.write(
        eCombinedImageSampler, 0u, 0u, shadowsTex, eShaderReadOnlyOptimal
    );
}

} // namespace rw
