/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/ShadowDrawer.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

constexpr int UNIT_MASK = ~(iLIGHT_SCALE * iTILEPx.x - 1);
constexpr int HALF_UNIT_OFFSET = iTILEPx.x * iLIGHT_SCALE / 2;

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

constexpr glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getAnalysisGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / LIGHT_SCALE), 1u};
}

constexpr glm::vec2 CALC_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getShadowsCalculationGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + LIGHT_SCALE * 2.0f) / CALC_GROUP_SIZE / LIGHT_SCALE), 1u};
}

ShadowDrawer::ShadowDrawer(const glm::uvec2& viewSizeTi, glm::uint maxNumberOfExternalLights):
    m_(viewSizeTi),
    m_analysisPll({}, RE::PipelineLayoutDescription{
        .bindings = {{
            {0u, eStorageImage, 1u, eCompute},          //lightImage
            {1u, eStorageImage, 1u, eCompute},          //transluImage
            {2u, eCombinedImageSampler, 1u, eCompute},  //worldSampler
            {3u, eCombinedImageSampler, 1u, eCompute},  //blockLightAtlas
            {4u, eCombinedImageSampler, 1u, eCompute},  //wallLightAtlas
            {5u, eStorageBuffer, 1u, eCompute},         //DynamicLightsSB
        }},
        .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(AnalysisPushConstants)}}
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
    m_drawingPll({}, {.vert = drawShadows_vert, .frag = drawColor_frag}),
    m_drawShadowsPl(
        {.pipelineLayout = *m_drawingPll},
        {.vert = drawShadows_vert, .frag = drawColor_frag}
    ) {
}

void ShadowDrawer::resizeView(const glm::uvec2& viewSizeTi) {
    m_ = ViewSizeDependent{viewSizeTi};
}

void ShadowDrawer::analyze(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& botLeftTi
) {
    /*pushConstants.analysisOffsetTi = (botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi)) & ~LIGHT_SCALE_BITS;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_analyzeTilesPl);
    commandBuffer.pushConstants<WorldDrawerPushConstants>(*pipelineLayout, eVertex | eFragment, 0u, pushConstants);
    commandBuffer.dispatch(m_.analysisGroupCount.x, m_.analysisGroupCount.y, m_.analysisGroupCount.z);
    m_lights.clear();*/
}

void ShadowDrawer::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    m_lights.emplace_back(posPx, col);
}

void ShadowDrawer::calculate(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& botLeftTi
) {
    //Add dynamic lights
    //m_lightsBuf.redefine(m_lights);
    //uniformBuf.overwrite(offsetof(WorldDrawerUniforms, addLightOffsetPx), ((botLeftPx - LIGHT_MAX_RANGETi * iTILEPx) & UNIT_MASK) + HALF_UNIT_OFFSET);
    ///uniformBuf.overwrite(offsetof(WorldDrawerUniforms, lightCount), glm::uint(m_lights.size()));
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    //m_addLightsPl.dispatchCompute(glm::uvec3{glm::ceil(m_lights.size() / 8.0f), 1u, 1u}, true);

    //Calculate shadows
    //RE::Ordering::issueIncoherentAccessBarrier(TEXTURE_FETCH);
    //commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_calculateShadowsPl);
    //commandBuffer.pushConstants<WorldDrawerPushConstants>(*pipelineLayout, eVertex | eFragment, 0u, pushConstants);
    //commandBuffer.dispatch(m_.calculationGroupCount.x, m_.calculationGroupCount.y, m_.calculationGroupCount.z);

}

void ShadowDrawer::draw(const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
    /*uniformBuf.overwrite(offsetof(WorldDrawerUniforms, drawShadowsReadOffsetTi), glm::ivec2(pxToTi(botLeftPx)) & LIGHT_SCALE_BITS);
    va.bind();
    m_drawShadowsPl.use();
    va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
    m_drawShadowsPl.unuse();
    va.unbind();*/
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(const glm::uvec2& viewSizeTi):
    ViewSizeDependent(getAnalysisGroupCount(viewSizeTi), getShadowsCalculationGroupCount(viewSizeTi)) {
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(const glm::uvec3& analysisGroupCount_, const glm::uvec3& calculationGroupCount_):
    lightTex(RE::TextureCreateInfo{
        .extent = {glm::vec2{analysisGroupCount_} *ANALYSIS_GROUP_SIZE, 1u}
    }),
    transluTex(RE::TextureCreateInfo{
        .format = vk::Format::eR8Unorm,
        .extent = {glm::vec2{analysisGroupCount_} *ANALYSIS_GROUP_SIZE, 1u}
    }),
    shadowsTex(RE::TextureCreateInfo{
        .extent = {glm::vec2{calculationGroupCount_} *ANALYSIS_GROUP_SIZE, 1u}
    }),
    analysisGroupCount(analysisGroupCount_),
    calculationGroupCount(calculationGroupCount_) {

    /*lightTex.bind(TEX_UNIT_LIGHT);
    transluTex.bind(TEX_UNIT_TRANSLU);
    shadowsTex.bind(TEX_UNIT_SHADOWS);
    lightTex.bindImage(IMG_UNIT_LIGHT, 0, RE::ImageAccess::READ_WRITE);
    transluTex.bindImage(IMG_UNIT_TRANSLU, 0, RE::ImageAccess::WRITE_ONLY);
    shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::WRITE_ONLY);*/
}
