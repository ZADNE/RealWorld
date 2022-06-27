/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/buffers/TypedBuffer.hpp>

#include <RealWorld/world/Light.hpp>
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

	struct ViewSizeDependent {
		ViewSizeDependent(const glm::uvec2& viewSizeTi);

		RE::Texture analysisTex;
		RE::Texture shadowsTex;
		RE::Texture pointLightCountTex;
		RE::TypedBuffer pointLightsBuf;
		glm::uvec3 analysisGroupCount;
		glm::uvec3 calcShadowsGroupCount;
	};

	ViewSizeDependent m_;

	RE::TexturePtr m_blockLightAtlasTex = RE::RM::getTexture("blockLightAtlas");
	RE::TexturePtr m_wallLightAtlasTex = RE::RM::getTexture("wallLightAtlas");

	RE::ShaderProgram m_analysisShd{{.comp = analysis_comp}};
	RE::ShaderProgram m_calcShadowsShd{{.comp = calcShadows_comp}};
	RE::ShaderProgram m_drawShadowsShd{{.vert = drawShadows_vert, .frag = colorDraw_frag}};
};
