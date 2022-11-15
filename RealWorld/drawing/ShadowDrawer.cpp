/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/ShadowDrawer.hpp>

#include <RealEngine/rendering/vertices/Vertex.hpp>
#include <RealEngine/rendering/output/Surface.hpp>
#include <RealEngine/rendering/Ordering.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>
#include <RealWorld/drawing/WorldDrawerUniforms.hpp>

constexpr int UNIT_MASK = ~(iLIGHT_SCALE * iTILEPx.x - 1);
constexpr int HALF_UNIT_OFFSET = iTILEPx.x * iLIGHT_SCALE / 2;

constexpr RE::TextureFlags R8_NU_NEAR_LIN_EDGE{
    RE::TextureChannels::R, RE::TextureFormat::NORMALIZED_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
    RE::TextureMagFilter::LINEAR, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
    RE::TextureBitdepthPerChannel::BITS_8
};

constexpr glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getAnalysisGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / LIGHT_SCALE), 1u};
}

constexpr glm::vec2 CALC_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getCalcShadowsGroupCount(const glm::vec2& viewSizeTi) {
    return {glm::ceil((viewSizeTi + LIGHT_SCALE * 2.0f) / CALC_GROUP_SIZE / LIGHT_SCALE), 1u};
}

template<RE::Renderer R>
ShadowDrawer<R>::ShadowDrawer(const glm::uvec2& viewSizeTi) :
    m_(viewSizeTi) {

    //Bind objects to their reserved texture units
    m_blockLightAtlasTex.bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
    m_wallLightAtlasTex.bind(TEX_UNIT_WALL_LIGHT_ATLAS);

    m_analyzeTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
    m_drawShadowsShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
    m_addLightsShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
    m_addLightsShd.backInterfaceBlock(0u, STRG_BUF_EXTERNALLIGHTS);
}

template<RE::Renderer R>
void ShadowDrawer<R>::resizeView(const glm::uvec2& viewSizeTi) {
    m_ = {viewSizeTi};
}

template<RE::Renderer R>
void ShadowDrawer<R>::analyze(const RE::BufferTyped<R>& uniformBuf, const glm::ivec2& botLeftTi) {
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, analysisOffsetTi), (botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi)) & ~LIGHT_SCALE_BITS);
    m_analyzeTilesShd.dispatchCompute(m_.analysisGroupCount, true);
    m_lights.clear();
}

template<RE::Renderer R>
void ShadowDrawer<R>::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    m_lights.emplace_back(posPx, col);
}

template<RE::Renderer R>
void ShadowDrawer<R>::calculate(const RE::BufferTyped<R>& uniformBuf, const glm::ivec2& botLeftPx) {
    using enum RE::IncoherentAccessBarrierFlags;

    //Add dynamic lights
    m_lightsBuf.redefine(m_lights);
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, addLightOffsetPx), ((botLeftPx - LIGHT_MAX_RANGETi * iTILEPx) & UNIT_MASK) + HALF_UNIT_OFFSET);
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, lightCount), glm::uint(m_lights.size()));
    RE::Ordering<R>::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_addLightsShd.dispatchCompute(glm::uvec3{glm::ceil(m_lights.size() / 8.0f), 1u, 1u}, true);

    //Calculate Shadows
    RE::Ordering<R>::issueIncoherentAccessBarrier(TEXTURE_FETCH);
    m_calcShadowsShd.dispatchCompute(m_.calcShadowsGroupCount, true);
}

template<RE::Renderer R>
void ShadowDrawer<R>::draw(const RE::BufferTyped<R>& uniformBuf, const RE::VertexArray<R>& va, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
    uniformBuf.overwrite(offsetof(WorldDrawerUniforms, drawShadowsReadOffsetTi), glm::ivec2(pxToTi(botLeftPx)) & LIGHT_SCALE_BITS);
    va.bind();
    m_drawShadowsShd.use();
    va.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
    m_drawShadowsShd.unuse();
    va.unbind();
}

template<RE::Renderer R>
ShadowDrawer<R>::ViewSizeDependent::ViewSizeDependent(const glm::uvec2& viewSizeTi) :
    lightTex({glm::vec2(getAnalysisGroupCount(viewSizeTi)) * ANALYSIS_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}),
    transluTex({glm::vec2(getAnalysisGroupCount(viewSizeTi)) * ANALYSIS_GROUP_SIZE}, {R8_NU_NEAR_LIN_EDGE}),
    shadowsTex({glm::vec2(getCalcShadowsGroupCount(viewSizeTi)) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}),
    analysisGroupCount(getAnalysisGroupCount(viewSizeTi)),
    calcShadowsGroupCount(getCalcShadowsGroupCount(viewSizeTi)) {

    lightTex.bind(TEX_UNIT_LIGHT);
    transluTex.bind(TEX_UNIT_TRANSLU);
    shadowsTex.bind(TEX_UNIT_SHADOWS);
    lightTex.bindImage(IMG_UNIT_LIGHT, 0, RE::ImageAccess::READ_WRITE);
    transluTex.bindImage(IMG_UNIT_TRANSLU, 0, RE::ImageAccess::WRITE_ONLY);
    shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::WRITE_ONLY);
}

template class ShadowDrawer<RE::RendererVK13>;
template class ShadowDrawer<RE::RendererGL46>;
