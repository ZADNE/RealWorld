/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/drawing.hpp>
#include <RealWorld/world/Light.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>

 /**
   * @brief Vertex with position and UVs
  */
struct VertexPOUV {
	VertexPOUV() {}

	VertexPOUV(const glm::vec2& position, const glm::vec2& uv) :
		position(position), uv(uv) {}

	glm::vec2 position;
	glm::vec2 uv;
};

/**
 * @brief Renders a world
 *
 * Renders its tiles and calculated shadows.
*/
class WorldDrawer {
public:
	WorldDrawer(const glm::uvec2& viewSizePx);
	~WorldDrawer();

	void setTarget(const glm::ivec2& worldDimTi);
	void resizeView(const glm::uvec2& newViewSizePx);

	struct ViewEnvelope {
		glm::vec2 botLeftTi;
		glm::vec2 topRightTi;
	};
	ViewEnvelope setPosition(const glm::vec2& botLeftPx);


	/**
	 * @brief Prepares for addition of lights
	*/
	void beginStep();

	/**
	 * @brief Adds a light
	 *
	 * All lights must be added between beginStep() and endStep()
	*/
	void addLight(const glm::vec2& posPx, RE::Color col);

	/**
	 * @brief Uploads lights
	*/
	void endStep();

	//Should be called at the beginning of draw beginStep
	void drawTiles();

	void shouldDrawShadows(bool should) { m_drawShadows = should; }
	void coverWithShadows();

	void shouldDrawMinimap(bool should) { m_drawMinimap = should; }
	void drawMinimap();
private:
	using enum RE::BufferStorage;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferAccessNature;
	using enum RE::BufferUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;

	void initVAOs();

	void updateArrayBuffers();

	void initShaders();
	void updateUniformsAfterViewResize();

	glm::vec2 m_invBotLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftTi;//Bottom-left corner of the view in blocks

	glm::vec2 m_viewSizePx;
	glm::uvec2 m_viewSizeTi;
	glm::uvec2 viewSizeTi(const glm::vec2& viewSizePx) const {
		return glm::uvec2(glm::ceil(viewSizePx / TILEPx)) + 1u;
	}

	glm::ivec2 m_worldDimTi;

	RE::TexturePtr m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	RE::TexturePtr m_wallAtlasTex = RE::RM::getTexture("wallAtlas");
	RE::TexturePtr m_blockLightAtlasTex = RE::RM::getTexture("blockLightAtlas");
	RE::TexturePtr m_wallLightAtlasTex = RE::RM::getTexture("wallLightAtlas");

	static inline const glm::vec2 ANALYSIS_GROUP_SIZE = glm::vec2{4.0f, 4.0f};
	static inline const glm::vec2 ANALYSIS_PER_THREAD_AREA = glm::vec2{4.0f, 4.0f};
	RE::ShaderProgram m_analysisShd{{.comp = analysis_comp}};
	glm::uvec3 m_analysisGroupCount;
	glm::uvec3 analysisGroupCount(const glm::vec2& viewSizeTi) {
		return {glm::ceil((viewSizeTi + glm::vec2(LIGHT_MAX_RANGETi) * 2.0f) / ANALYSIS_GROUP_SIZE / ANALYSIS_PER_THREAD_AREA), 1u};
	}
	RE::Texture m_analysisTex;

	static inline const glm::vec2 CALC_GROUP_SIZE = glm::vec2{4.0f, 4.0f};
	RE::ShaderProgram m_calcShadowsShd{{.comp = calcShadows_comp}};
	glm::uvec3 m_calcShadowsGroupCount;
	glm::uvec3 calcShadowsGroupCount(const glm::vec2& viewSizeTi) {
		return {glm::ceil(viewSizeTi / CALC_GROUP_SIZE), 1u};
	}
	RE::Texture m_shadowsTex;

	RE::ShaderProgram m_drawTilesShd{{.vert = drawTiles_vert, .frag = colorDraw_frag}};
	RE::ShaderProgram m_drawShadowsShd{{.vert = drawShadows_vert, .frag = colorDraw_frag}};
	RE::ShaderProgram m_minimapShd{{.vert = minimap_vert, .frag = minimap_frag}};
	bool m_drawShadows = true;
	bool m_drawMinimap = false;


	RE::VertexArray m_pouvArr;

	RE::Buffer m_pouvBuf{sizeof(VertexPOUV) * 8, DYNAMIC_STORAGE};

	struct WorldDrawUniforms {
		glm::mat4 viewsizePxMat;
	};
	RE::TypedBuffer m_worldDrawUniformBuf{UNIF_BUF_WORLDDRAWER, sizeof(WorldDrawUniforms), DYNAMIC_STORAGE};
};