/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/buffers/TypedBuffer.hpp>

#include <RealWorld/shaders/drawing.hpp>

 /**
  * @brief Renders shadows of the world
 */
class ShadowDrawer {
public:
	ShadowDrawer(const glm::uvec2& viewSizeTi, RE::TypedBuffer& uniformBuf);
	~ShadowDrawer();

	void resizeView(const glm::uvec2& viewSizeTi);

	void analyze(const glm::ivec2& botLeftTi);

	void calculate();

	void draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);
private:
	RE::TexturePtr m_blockLightAtlasTex = RE::RM::getTexture("blockLightAtlas");
	RE::TexturePtr m_wallLightAtlasTex = RE::RM::getTexture("wallLightAtlas");

	static inline const glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{4.0f, 4.0f};
	static inline const glm::vec2 ANALYSIS_PER_THREAD_AREA = glm::vec2{4.0f, 4.0f};
	RE::ShaderProgram m_analysisShd{{.comp = analysis_comp}};
	glm::uvec3 m_analysisGroupCount;
	glm::uvec3 analysisGroupCount(const glm::vec2& viewSizeTi) const;
	RE::Texture m_analysisTex;

	static inline const glm::vec2 CALC_GROUP_SIZE = glm::vec2{4.0f, 4.0f};
	RE::ShaderProgram m_calcShadowsShd{{.comp = calcShadows_comp}};
	glm::uvec3 m_calcShadowsGroupCount;
	glm::uvec3 calcShadowsGroupCount(const glm::vec2& viewSizeTi) const;

	RE::Texture m_shadowsTex;

	void bindTexturesAndImages();

	RE::ShaderProgram m_drawShadowsShd{{.vert = drawShadows_vert, .frag = colorDraw_frag}};
};