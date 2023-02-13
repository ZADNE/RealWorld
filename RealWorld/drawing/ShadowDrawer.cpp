/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/ShadowDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

constexpr int UNIT_MASK = ~(iLIGHT_SCALE * iTILEPx.x - 1);
constexpr int HALF_UNIT_OFFSET = iTILEPx.x * iLIGHT_SCALE / 2;

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using enum vk::MemoryPropertyFlagBits;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

constexpr glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getAnalysisGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / LIGHT_SCALE), 1u};
}

constexpr glm::vec2 CALC_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getShadowsCalculationGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + LIGHT_SCALE * 2.0f) / CALC_GROUP_SIZE / LIGHT_SCALE), 1u};
}

ShadowDrawer::ShadowDrawer(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi, glm::uint maxNumberOfExternalLights):
    m_analysisPll({}, RE::PipelineLayoutDescription{
        .bindings = {{
            {0u, eStorageImage, 1u, eCompute},          //lightImage
            {1u, eStorageImage, 1u, eCompute},          //transluImage
            {2u, eCombinedImageSampler, 1u, eCompute},  //worldSampler
            {3u, eCombinedImageSampler, 1u, eCompute},  //blockLightAtlas
            {4u, eCombinedImageSampler, 1u, eCompute},  //wallLightAtlas
            {5u, eStorageBuffer, 1u, eCompute},         //DynamicLightsSB
        }},
        .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(AnalysisPC)}}
    }),
    m_analyzeTilesPl(
        {.pipelineLayout = *m_analysisPll},
        {.comp = analyzeTiles_comp}
    ),
    m_addLightsPl(
        {.pipelineLayout = *m_analysisPll},
        {.comp = addDynamicLights_comp}
    ),
    m_calculationPll({}, RE::PipelineLayoutDescription{
        .bindings = {{
            {0u, eCombinedImageSampler, 1u, eCompute},  //lightSampler
            {1u, eCombinedImageSampler, 1u, eCompute},  //transluSampler
            {2u, eStorageImage, 1u, eCompute},          //shadowsImage
        }}
    }),
    m_calculateShadowsPl(
        {.pipelineLayout = *m_calculationPll},
        {.comp = calculateShadows_comp}
    ),
    m_shadowDrawingPll({}, {.vert = drawShadows_vert, .frag = drawColor_frag}),
    m_drawShadowsPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *m_shadowDrawingPll,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, {
            .vert = drawShadows_vert,
            .frag = drawColor_frag
        }
    ),
    m_lightsBuf(
        maxNumberOfExternalLights * sizeof(ExternalLight),
        vk::BufferUsageFlagBits::eStorageBuffer,
        eHostVisible | eHostCoherent
    ),
    m_lightsMapped(m_lightsBuf.map<ExternalLight>(0ull, VK_WHOLE_SIZE)),
    m_(viewSizePx, viewSizeTi, m_analysisPll, m_calculationPll, m_shadowDrawingPll,
       m_blockLightAtlasTex, m_wallLightAtlasTex, m_lightsBuf
    ) {
}

void ShadowDrawer::setTarget(const RE::Texture& worldTexture, const glm::ivec2& worldTexSize) {
    m_.analysisPC.worldTexMask = worldTexSize - 1;
    m_.analysisDS.write(eCombinedImageSampler, 2u, 0u, worldTexture, eReadOnlyOptimal);
}

void ShadowDrawer::resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi) {
    m_ = ViewSizeDependent{
        viewSizePx, viewSizeTi, m_analysisPll, m_calculationPll, m_shadowDrawingPll,
        m_blockLightAtlasTex, m_wallLightAtlasTex, m_lightsBuf
    };
}

void ShadowDrawer::analyze(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& botLeftTi
) {
    m_.analysisPC.analysisOffsetTi = (botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi)) & ~LIGHT_SCALE_BITS;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeTilesPl);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_analysisPll, 0u, *m_.analysisDS, {});
    commandBuffer.pushConstants<AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
    commandBuffer.dispatch(m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z);
    m_.analysisPC.lightCount = 0;
}

void ShadowDrawer::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    m_lightsMapped[m_.analysisPC.lightCount++] = ExternalLight{posPx, col};
}

void ShadowDrawer::calculate(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& botLeftPx
) {
    //Add dynamic lights
    if (m_.analysisPC.lightCount > 0) {
        m_.analysisPC.addLightOffsetPx = ((botLeftPx - LIGHT_MAX_RANGETi * iTILEPx) & UNIT_MASK) + HALF_UNIT_OFFSET;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_addLightsPl);
        commandBuffer.pushConstants<AnalysisPC>(*m_analysisPll, eCompute, 0u, m_.analysisPC);
        commandBuffer.dispatch(1u + (m_.analysisPC.lightCount - 1u) / 8u, 1u, 1u);
    }

    //Calculate shadows
    vk::ImageMemoryBarrier2 imageBarriers[] = {
        vk::ImageMemoryBarrier2{
            S::eComputeShader,                                                          //Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite,                             //Src access mask
            S::eComputeShader,                                                          //Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite,                             //Dst access mask
            eGeneral,                                                                   //Old image layout
            eShaderReadOnlyOptimal,                                                     //New image layout
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
            m_.lightTex.image(),
            vk::ImageSubresourceRange{eColor, 0u, 1u, 0u, 1u}
        },
        vk::ImageMemoryBarrier2{
            S::eComputeShader,                                                          //Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite,                             //Src access mask
            S::eComputeShader,                                                          //Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite,                             //Dst access mask
            eGeneral,                                                                   //Old image layout
            eShaderReadOnlyOptimal,                                                     //New image layout
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
            m_.transluTex.image(),
            vk::ImageSubresourceRange{eColor, 0u, 1u, 0u, 1u}
        },
        vk::ImageMemoryBarrier2{
            S::eVertexShader,                                                           //Src stage mask
            A::eShaderSampledRead,                                                      //Src access mask
            S::eComputeShader,                                                          //Dst stage mask
            A::eShaderStorageWrite,                                                     //Dst access mask
            eShaderReadOnlyOptimal,                                                     //Old image layout
            eGeneral,                                                                   //New image layout
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
            m_.shadowsTex.image(),
            vk::ImageSubresourceRange{eColor, 0u, 1u, 0u, 1u}
        }
    };
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_calculateShadowsPl);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_calculationPll, 0u, *m_.calculationDS, {});
    commandBuffer.dispatch(m_.calculationGroupCount.x, m_.calculationGroupCount.y, m_.calculationGroupCount.z);
    std::swap(imageBarriers[0].oldLayout, imageBarriers[0].newLayout);
    std::swap(imageBarriers[1].oldLayout, imageBarriers[1].newLayout);
    std::swap(imageBarriers[2].oldLayout, imageBarriers[2].newLayout);
    std::swap(imageBarriers[2].srcStageMask, imageBarriers[2].dstStageMask);
    std::swap(imageBarriers[2].srcStageMask, imageBarriers[2].dstStageMask);
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarriers});

}

void ShadowDrawer::draw(
    const vk::CommandBuffer& commandBuffer,
    const glm::vec2& botLeftPx,
    const glm::uvec2& viewSizeTi
) {
    m_.shadowDrawingPC.botLeftPxModTilePx = glm::mod(botLeftPx, TILEPx);
    m_.shadowDrawingPC.readOffsetTi = glm::ivec2(pxToTi(botLeftPx)) & LIGHT_SCALE_BITS;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawShadowsPl);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_shadowDrawingPll, 0u, *m_.shadowDrawingDS, {});
    commandBuffer.pushConstants<ShadowDrawingPC>(*m_shadowDrawingPll, eVertex, 0u, m_.shadowDrawingPC);
    commandBuffer.draw(4u, viewSizeTi.x * viewSizeTi.y, 0u, 0u);
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(
    const glm::vec2& viewSizePx,
    const glm::ivec2& viewSizeTi,
    const RE::PipelineLayout& analysisPll,
    const RE::PipelineLayout& calculationPll,
    const RE::PipelineLayout& shadowDrawingPll,
    const RE::Texture& blockLightAtlasTex,
    const RE::Texture& wallLightAtlasTex,
    const RE::Buffer& lightsBuf
):
    analysisGroupCount(getAnalysisGroupCount(viewSizeTi)),
    calculationGroupCount(getShadowsCalculationGroupCount(viewSizeTi)),
    lightTex(RE::TextureCreateInfo{
        .extent = {glm::vec2{analysisGroupCount} *ANALYSIS_GROUP_SIZE, 1u},
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
        .initialLayout = eGeneral,
        .magFilter = vk::Filter::eLinear
    }),
    transluTex(RE::TextureCreateInfo{
        .format = vk::Format::eR8Unorm,
        .extent = {glm::vec2{analysisGroupCount} *ANALYSIS_GROUP_SIZE, 1u},
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
        .initialLayout = eGeneral,
        .magFilter = vk::Filter::eLinear
    }),
    shadowsTex(RE::TextureCreateInfo{
        .extent = {glm::vec2{calculationGroupCount} *CALC_GROUP_SIZE, 1u},
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
        .magFilter = vk::Filter::eLinear
    }),
    shadowDrawingPC({
        .viewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y),
        .viewSizeTi = viewSizeTi
    }),
    analysisDS(analysisPll, 0u),
    calculationDS(calculationPll, 0u),
    shadowDrawingDS(shadowDrawingPll, 0u) {
    using enum vk::DescriptorType;
    //Analysis descriptor set
    analysisDS.write(eStorageImage, 0u, 0u, lightTex, eGeneral);
    analysisDS.write(eStorageImage, 1u, 0u, transluTex, eGeneral);
    analysisDS.write(eCombinedImageSampler, 3u, 0u, blockLightAtlasTex, eReadOnlyOptimal);
    analysisDS.write(eCombinedImageSampler, 4u, 0u, wallLightAtlasTex, eReadOnlyOptimal);
    analysisDS.write(eStorageBuffer, 5u, 0u, lightsBuf, 0ull, VK_WHOLE_SIZE);
    //Calculation descriptor set
    calculationDS.write(eCombinedImageSampler, 0u, 0u, lightTex, eReadOnlyOptimal);
    calculationDS.write(eCombinedImageSampler, 1u, 0u, transluTex, eReadOnlyOptimal);
    calculationDS.write(eStorageImage, 2u, 0u, shadowsTex, eGeneral);
    //Shadow drawing descriptor set
    shadowDrawingDS.write(eCombinedImageSampler, 0u, 0u, shadowsTex, eReadOnlyOptimal);
}
