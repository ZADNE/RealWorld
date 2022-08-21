/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vector>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>

#include <RealWorld/drawing/ExternalLight.hpp>
#include <RealWorld/shaders/drawing.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

 /**
  * @brief Renders shadows of the world
 */
class ShadowDrawer {
public:
	ShadowDrawer(const glm::uvec2& viewSizeTi);

	void resizeView(const glm::uvec2& viewSizeTi);

	void analyze(const glm::ivec2& botLeftTi);

	void addExternalLight(const glm::ivec2& posPx, RE::Color col);

	void calculate(const glm::ivec2& botLeftPx);

	void draw(const RE::VertexArray& vao, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);
private:

	struct ViewSizeDependent {
		ViewSizeDependent(const glm::uvec2& viewSizeTi);

		RE::Texture lightTex;//RGB = color of the light, A = intensity of the light
		RE::Texture transluTex;//R = translucency of the unit
		RE::Texture shadowsTex;
		glm::uvec3 analysisGroupCount;
		glm::uvec3 calcShadowsGroupCount;
	};

	ViewSizeDependent m_;

	RE::TexturePtr m_blockLightAtlasTex = RE::RM::getTexture("blockLightAtlas");
	RE::TexturePtr m_wallLightAtlasTex = RE::RM::getTexture("wallLightAtlas");

	RE::ShaderProgram m_analysisShd{ {.comp = analysis_comp} };
	RE::ShaderProgram m_addLightsShd{ {.comp = addDynamicLights_comp} };
	RE::ShaderProgram m_calcShadowsShd{ {.comp = calcShadows_comp} };
	RE::ShaderProgram m_drawShadowsShd{ {.vert = drawShadows_vert, .frag = colorDraw_frag} };

	std::vector<ExternalLight> m_lights;
	RE::TypedBuffer m_lightsBuf{ STRG_BUF_EXTERNALLIGHTS, sizeof(ExternalLight) * 8, RE::BufferAccessFrequency::DYNAMIC, RE::BufferAccessNature::DRAW, nullptr };
};
