#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/TextureCache.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/shaders/WDS.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/world/DynamicLight.hpp>
#include <RealWorld/world/LightManipulator.hpp>
#include <RealWorld/rendering/Vertex.hpp>
#include <RealWorld/rendering/UniformBuffers.hpp>

class WorldDrawer {
	friend class LightManipulator;
public:
	WorldDrawer(const glm::uvec2& viewSizePx);
	~WorldDrawer();

	void setTarget(const glm::ivec2& worldDimBc);
	void resizeView(const glm::uvec2& newViewSizePx);

	//For adding and removing lights
	LightManipulator getLightManipulator();

	//All dynamic lights must be added each between beginStep() and endStep()
	void beginStep(const glm::vec2& botLeftPx, World& world);
	//All dynamic lights must be added each between beginStep() and endStep()
	void endStep();

	//Should be called at the beginning of draw beginStep
	void drawTiles();
	//Should be called at the end of draw beginStep (to cover everything else with darkness) but before GUI
	void coverWithDarkness();

	void toggleMinimap() { m_drawMinimap = !m_drawMinimap; }
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


	void addLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone);


	void reloadViewSize(const glm::uvec2& viewSizePx);

	void initVAOs();

	void updatePOUVBuffers();

	void initShaders();
	void updateUniformsAfterViewResize();

	RE::Surface m_SurLighting{{RE::TextureFlags::RGBA_NU_NEAR_NEAR_EDGE}, false, false};
	//0 texture (diaphragm):			R = diaphragm of the tile; G = direction of the light; B = width of light cone; A = unused yet
	//1 texture (lights):				RGB = colour of the light; A = strength of the light
	//2 texture (finished lighting):	RGBA = drawn to screen

	glm::vec2 m_botLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftBc;//Bottom-left corner of the view in blocks
	glm::vec2 m_viewsizePx;
	glm::uvec2 m_viewsizeBc;
	glm::uvec2 m_viewsizeLightingBc;

	glm::ivec2 m_worldDimBc;

	RE::TexturePtr m_blockAtlasTex;
	RE::TexturePtr m_wallAtlasTex;

	RE::ShaderProgram m_tilesShader = RE::ShaderProgram({.vert = WDS::tilesDraw_vert, .frag = WDS::tilesDraw_frag});
	RE::ShaderProgram m_coverWithDarknessShader = RE::ShaderProgram({.vert = WDS::finalLighting_vert, .frag = shaders::texture_frag});
	RE::ShaderProgram m_computeLightingShader = RE::ShaderProgram({.vert = WDS::PT_vert, .frag = WDS::combineLighting_frag});//Combines diaphragm with lights
	RE::ShaderProgram m_worldToLightsShader = RE::ShaderProgram({.vert = WDS::PT_vert, .frag = WDS::worldToLight_frag});//Processes world texture to light

	RE::ShaderProgram m_addDynamicLightShader = RE::ShaderProgram({.vert = WDS::addDynamicLight_vert, .frag = WDS::addDynamicLight_frag});

	RE::ShaderProgram m_minimapShader = RE::ShaderProgram{{.vert = shaders::data_vert, .frag = shaders::worldDebug_frag}};
	bool m_drawMinimap = false;


	RE::VertexArray m_arrayPOUV;
	RE::VertexArray m_arrayLights;

	RE::Buffer<ARRAY, IMMUTABLE> m_bufferPOUV{sizeof(VertexPOUV) * 16, DYNAMIC_STORAGE};
	RE::Buffer<ARRAY, MUTABLE> m_bufferDynamicLights{STREAM, DRAW};

	struct WorldDrawUniforms {
		glm::mat4 viewsizePxMat;
		glm::mat4 viewsizeLightingBcMat;
	};
	RE::UniformBuffer m_worldDrawUniformBuffer{UNIF_BUF_WORLDDRAWER, true, DYNAMIC_STORAGE, sizeof(WorldDrawUniforms)};

	//Lighting global
	float m_dayLength = 200.0f;
	float m_currentTime = 50.0f;
	glm::vec4 m_backgroundColour = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

	//Dynamic lights
	std::vector<DynamicLight> m_dynamicLights;
};