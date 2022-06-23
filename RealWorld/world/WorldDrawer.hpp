/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/drawing.hpp>
#include <RealWorld/world/Light.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

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
	using enum RE::BufferType;
	using enum RE::BufferStorage;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferAccessNature;
	using enum RE::BufferUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;

	void reloadViewSize(const glm::uvec2& viewSizePx);

	void initVAOs();

	void updatePOUVBuffers();

	void initShaders();
	void updateUniformsAfterViewResize();

	static inline const RE::TextureFlags RGBA32_NU_NEAR_LIN_EDGE{
		RE::TextureChannels::RGBA, RE::TextureFormat::NORMALIZED_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
		RE::TextureMagFilter::LINEAR, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
		RE::TextureBitdepthPerChannel::BITS_8
	};

	RE::Surface m_surLighting{{RGBA32_NU_NEAR_LIN_EDGE}, false, false};
	//0 texture (light):	RGB = color of the light at the tile; A = strength of the light
	//1 texture (translu):	RGB = unused; A = translucence of the tile
	//2 texture (computed):	RGBA = drawn to screen

	glm::vec2 m_invBotLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftTi;//Bottom-left corner of the view in blocks

	glm::vec2 m_viewDimsPx;
	glm::uvec2 m_viewDimsTi;
	glm::uvec2 m_viewDimsUn;//Units that overlap the view

	glm::ivec2 m_worldDimTi;

	RE::TexturePtr m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	RE::TexturePtr m_wallAtlasTex = RE::RM::getTexture("wallAtlas");
	RE::TexturePtr m_blockLightAtlasTex = RE::RM::getTexture("blockLightAtlas");
	RE::TexturePtr m_wallLightAtlasTex = RE::RM::getTexture("wallLightAtlas");

	RE::ShaderProgram m_tilesShader{{.vert = tilesDraw_vert, .frag = colorDraw_frag}};

	RE::ShaderProgram m_coverWithShadowsShader{{.vert = coverWithShadows_vert, .frag = colorDraw_frag}};//Stretches light texture to whole screen


	RE::ShaderProgram m_minimapShader{{.vert = minimap_vert, .frag = minimap_frag}};
	bool m_drawShadows = true;
	bool m_drawMinimap = false;


	RE::VertexArray m_pouvArray;

	RE::Buffer m_pouvBuffer{sizeof(VertexPOUV) * 8, DYNAMIC_STORAGE};

	struct WorldDrawUniforms {
		glm::mat4 viewsizePxMat;
	};
	RE::TypedBuffer m_worldDrawUniformBuffer{UNIFORM, UNIF_BUF_WORLDDRAWER, sizeof(WorldDrawUniforms), DYNAMIC_STORAGE};
};