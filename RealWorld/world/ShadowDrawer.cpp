/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ShadowDrawer.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

const RE::TextureFlags R8_NU_NEAR_LIN_EDGE{
	RE::TextureChannels::R, RE::TextureFormat::NORMALIZED_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
	RE::TextureMagFilter::LINEAR, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
	RE::TextureBitdepthPerChannel::BITS_8
};

const RE::TextureFlags R32_IU_NEAR_NEAR_EDGE{
	RE::TextureChannels::R, RE::TextureFormat::INTEGRAL_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
	RE::TextureMagFilter::NEAREST, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
	RE::TextureBitdepthPerChannel::BITS_32
};

constexpr glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{8.0f};
constexpr glm::vec2 ANALYSIS_PER_THREAD_AREA = glm::vec2{4.0f};
glm::uvec3 getAnalysisGroupCount(const glm::vec2& viewSizeTi) {
	return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / ANALYSIS_PER_THREAD_AREA), 1u};
}

constexpr glm::vec2 CALC_GROUP_SIZE = glm::vec2{8.0f};
glm::uvec3 getCalcShadowsGroupCount(const glm::vec2& viewSizeTi) {
	return {glm::ceil(viewSizeTi / CALC_GROUP_SIZE), 1u};
}

struct PointLight {
	glm::uint posTi;//2 half floats - X and Y pos
	RE::Color col;//RGB = color of the light, A = strength of the light (15 is max for single tile!)
};
GLsizeiptr getPointLightsBufSize(const glm::uvec2& analysisGroupCount) {
	constexpr auto GS = glm::uvec2(ANALYSIS_GROUP_SIZE);
	return sizeof(PointLight) * analysisGroupCount.x * analysisGroupCount.y * GS.x * GS.y;
}


ShadowDrawer::ShadowDrawer(const glm::uvec2& viewSizeTi, RE::TypedBuffer& uniformBuf) :
	m_(viewSizeTi) {

	//Bind objects to their reserved texture units
	m_blockLightAtlasTex->bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
	m_wallLightAtlasTex->bind(TEX_UNIT_WALL_LIGHT_ATLAS);

	uniformBuf.connectToInterfaceBlock(m_analysisShd, 0u);
	uniformBuf.connectToInterfaceBlock(m_drawShadowsShd, 0u);

	m_.pointLightsBuf.connectToInterfaceBlock(m_analysisShd, 0u);
	m_.pointLightsBuf.connectToInterfaceBlock(m_calcShadowsShd, 0u);
	m_calcShadowsShd.setUniform("slotsInRow", m_.analysisGroupCount.x);
}

ShadowDrawer::~ShadowDrawer() {

}

void ShadowDrawer::resizeView(const glm::uvec2& viewSizeTi) {
	m_ = {viewSizeTi};

	m_.pointLightsBuf.connectToInterfaceBlock(m_analysisShd, 0u);
	m_.pointLightsBuf.connectToInterfaceBlock(m_calcShadowsShd, 0u);
	m_calcShadowsShd.setUniform("slotsInRow", m_.analysisGroupCount.x);
}

void ShadowDrawer::analyze(const glm::ivec2& botLeftTi) {
	m_.pointLightCountTex.clear(glm::uvec4(0u));
	m_analysisShd.setUniform(LOC_POSITIONTi, botLeftTi);
	m_analysisShd.dispatchCompute(m_.analysisGroupCount, true);
}

void ShadowDrawer::calculate() {
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	m_calcShadowsShd.dispatchCompute(m_.calcShadowsGroupCount, true);
}

void ShadowDrawer::draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
	vao.bind();
	m_drawShadowsShd.use();
	m_drawShadowsShd.setUniform(LOC_POSITIONPx, glm::mod(botLeftPx, TILEPx));
	vao.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
	m_drawShadowsShd.unuse();
	vao.unbind();
}

ShadowDrawer::ViewSizeDependent::ViewSizeDependent(const glm::uvec2& viewSizeTi) :
	analysisTex({glm::vec2(getAnalysisGroupCount(viewSizeTi)) * ANALYSIS_GROUP_SIZE}, {R8_NU_NEAR_LIN_EDGE}),
	shadowsTex({glm::vec2(getCalcShadowsGroupCount(viewSizeTi)) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}),
	pointLightCountTex({getAnalysisGroupCount(viewSizeTi)}, {R32_IU_NEAR_NEAR_EDGE}),
	pointLightsBuf(STRG_BUF_POINTLIGHTS, getPointLightsBufSize(getAnalysisGroupCount(viewSizeTi)), RE::BufferUsageFlags::NO_FLAGS),
	analysisGroupCount(getAnalysisGroupCount(viewSizeTi)),
	calcShadowsGroupCount(getCalcShadowsGroupCount(viewSizeTi)) {

	analysisTex.bind(TEX_UNIT_TILE_TRANSLU);
	shadowsTex.bind(TEX_UNIT_SHADOWS);
	pointLightCountTex.bind(TEX_UNIT_POINT_LIGHT_COUNT);
	analysisTex.bindImage(IMG_UNIT_TILE_TRANSLU, 0, RE::ImageAccess::READ_WRITE);
	shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::READ_WRITE);
	pointLightCountTex.bindImage(IMG_UNIT_POINT_LIGHT_COUNT, 0, RE::ImageAccess::READ_WRITE);
}
