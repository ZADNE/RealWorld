/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ShadowDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/chunk/ChunkManager.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/shaders/common.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

static inline const RE::TextureFlags R8_NU_NEAR_LIN_EDGE{
	RE::TextureChannels::R, RE::TextureFormat::NORMALIZED_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
	RE::TextureMagFilter::LINEAR, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
	RE::TextureBitdepthPerChannel::BITS_8
};


ShadowDrawer::ShadowDrawer(const glm::uvec2& viewSizeTi, RE::TypedBuffer& uniformBuf) :
	m_analysisGroupCount(analysisGroupCount(viewSizeTi)),
	m_analysisTex({glm::vec2(m_analysisGroupCount) * ANALYSIS_GROUP_SIZE * ANALYSIS_PER_THREAD_AREA}, {R8_NU_NEAR_LIN_EDGE}),
	m_calcShadowsGroupCount(calcShadowsGroupCount(viewSizeTi)),
	m_shadowsTex({glm::vec2(m_calcShadowsGroupCount) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}) {

	//Bind textures & images to their reserved texture units
	m_blockLightAtlasTex->bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
	m_wallLightAtlasTex->bind(TEX_UNIT_WALL_LIGHT_ATLAS);

	bindTexturesAndImages();

	uniformBuf.connectToInterfaceBlock(m_drawShadowsShd, 0u);
}

ShadowDrawer::~ShadowDrawer() {

}

void ShadowDrawer::resizeView(const glm::uvec2& viewSizeTi) {
	m_analysisGroupCount = analysisGroupCount(viewSizeTi);
	m_analysisTex = RE::Texture({glm::vec2(m_analysisGroupCount) * ANALYSIS_GROUP_SIZE * ANALYSIS_PER_THREAD_AREA}, {R8_NU_NEAR_LIN_EDGE});
	m_calcShadowsGroupCount = calcShadowsGroupCount(viewSizeTi);
	m_shadowsTex = RE::Texture({glm::vec2(m_calcShadowsGroupCount) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE});

	bindTexturesAndImages();
}

void ShadowDrawer::analyze(const glm::ivec2& botLeftTi) {
	m_analysisShd.setUniform(LOC_POSITION, botLeftTi);
	m_analysisShd.dispatchCompute(m_analysisGroupCount, true);
}

void ShadowDrawer::calculate() {
	m_calcShadowsShd.dispatchCompute(m_calcShadowsGroupCount, true);
}

void ShadowDrawer::draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
	vao.bind();
	m_drawShadowsShd.use();
	m_drawShadowsShd.setUniform(LOC_POSITION, glm::mod(botLeftPx, TILEPx));
	vao.renderArrays(RE::Primitive::TRIANGLE_STRIP, 0, 4, viewSizeTi.x * viewSizeTi.y);
	m_drawShadowsShd.unuse();
	vao.unbind();
}

glm::uvec3 ShadowDrawer::analysisGroupCount(const glm::vec2& viewSizeTi) const {
	return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / ANALYSIS_PER_THREAD_AREA), 1u};
}

glm::uvec3 ShadowDrawer::calcShadowsGroupCount(const glm::vec2& viewSizeTi) const {
	return {glm::ceil(viewSizeTi / CALC_GROUP_SIZE), 1u};
}

void ShadowDrawer::bindTexturesAndImages() {
	m_analysisTex.bind(TEX_UNIT_TILE_TRANSLU);
	m_shadowsTex.bind(TEX_UNIT_SHADOWS);
	m_analysisTex.bindImage(IMG_UNIT_TILE_TRANSLU, 0, RE::ImageAccess::READ_WRITE);
	m_shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::READ_WRITE);
}
