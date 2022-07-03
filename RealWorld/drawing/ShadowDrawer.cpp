/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/ShadowDrawer.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

constexpr int UNIT_MASK = ~(iLIGHT_SCALE * iTILEPx.x - 1);
constexpr int HALF_UNIT_OFFSET = iTILEPx.x * iLIGHT_SCALE / 2;

const RE::TextureFlags R8_NU_NEAR_LIN_EDGE{
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

ShadowDrawer::ShadowDrawer(const glm::uvec2& viewSizeTi) :
	m_(viewSizeTi) {

	//Bind objects to their reserved texture units
	m_blockLightAtlasTex->bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
	m_wallLightAtlasTex->bind(TEX_UNIT_WALL_LIGHT_ATLAS);

	m_analysisShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
	m_drawShadowsShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDRAWER);
	m_addLightsShd.backInterfaceBlock(0u, STRG_BUF_EXTERNALLIGHTS);
}

ShadowDrawer::~ShadowDrawer() {

}

void ShadowDrawer::resizeView(const glm::uvec2& viewSizeTi) {
	m_ = {viewSizeTi};
}

void ShadowDrawer::analyze(const glm::ivec2& botLeftTi) {
	m_analysisShd.setUniform(LOC_POSITIONTi, (botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi)) & ~LIGHT_SCALE_BITS);
	m_analysisShd.dispatchCompute(m_.analysisGroupCount, true);
	m_lights.clear();
}

void ShadowDrawer::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
	m_lights.emplace_back(posPx, col);
}

void ShadowDrawer::calculate(const glm::ivec2& botLeftPx) {
	//Add dyanmic lights
	m_lightsBuf.redefine(m_lights);
	m_addLightsShd.setUniform(LOC_LIGHT_COUNT, glm::uint(m_lights.size()));
	glm::ivec2 viewBotLeftPx = (botLeftPx - LIGHT_MAX_RANGETi * iTILEPx) & UNIT_MASK;
	m_addLightsShd.setUniform(LOC_POSITIONPx, viewBotLeftPx + HALF_UNIT_OFFSET);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	m_addLightsShd.dispatchCompute(glm::uvec3{glm::ceil(m_lights.size() / 8.0f), 1u, 1u}, true);
	//Calculate Shadows
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	m_calcShadowsShd.dispatchCompute(m_.calcShadowsGroupCount, true);
}

void ShadowDrawer::draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
	vao.bind();
	m_drawShadowsShd.use();
	m_drawShadowsShd.setUniform(LOC_POSITIONPx, glm::mod(botLeftPx, TILEPx));
	glm::ivec2 botLeftTi = pxToTi(botLeftPx);
	m_drawShadowsShd.setUniform(LOC_POSITIONTi, botLeftTi & LIGHT_SCALE_BITS);
	vao.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
	m_drawShadowsShd.unuse();
	vao.unbind();
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(const glm::uvec2& viewSizeTi) :
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
