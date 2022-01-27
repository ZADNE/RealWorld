#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/shaders/WDS.hpp>
#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/world/DynamicLight.hpp>
#include <RealWorld/world/LightManipulator.hpp>
#include <RealWorld/rendering/Vertex.hpp>

class WorldDrawer {
	friend class LightManipulator;
public:
	WorldDrawer(const glm::uvec2& viewSizePx, const glm::mat4& viewMatrix, ChunkManager& chunkHandler);
	~WorldDrawer();

	void setTarget(const glm::ivec2& worldDimBc, RE::TextureProxy worldTexture);
	void resizeView(const glm::uvec2& newViewSizePx, const glm::mat4& newViewMatrix);

	//For adding and removing lights
	LightManipulator getLightManipulator();

	//All dynamic lights must be added each between beginStep() and endStep()
	void beginStep(const glm::vec2& botLeftPx);
	//All dynamic lights must be added each between beginStep() and endStep()
	void endStep();

	//Should be called at the beginning of draw step
	void drawWorld();
	//Should be called at the end of draw step (to cover everything else with darkness) but before GUI
	void coverWithDarkness();
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

	void initShaders(const glm::mat4& viewMatrix);
	void updateUniformsAfterResize(const glm::mat4& newViewMatrix);

	RE::Surface m_SurLighting{{RE::TextureFlags::RGBA_NU_NEAR_NEAR_EDGE}, false, false};
	//0 texture (diaphragm):			R = diaphragm of the tile; G = direction of the light; B = width of light cone; A = unused yet
	//1 texture (lighting):				RGB = colour of the light; A = strength of the light
	//2 texture (finished lighting):	RGBA = drawn without change to screen

	glm::vec2 m_botLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftBc;//Bottom-left corner of the view in blocks
	glm::vec2 m_viewsizePx;
	glm::uvec2 m_viewsizeBc;
	glm::uvec2 m_viewsizeLightingBc;
	ChunkManager& m_chunkHandler;

	glm::ivec2 m_worldDimBc;

	RE::TexturePtr m_blockAtlasTex;
	RE::TexturePtr m_wallAtlasTex;

	RE::ShaderProgramPtr m_tilesShader = RE::RM::getShaderProgram({.vert = WDS::tilesDraw_vert, .frag = WDS::tilesDraw_frag});
	RE::ShaderProgramPtr m_coverWithDarknessShader = RE::RM::getShaderProgram({.vert = WDS::finalLighting_vert, .frag = shaders::texture_frag});
	RE::ShaderProgramPtr m_computeLightingShader = RE::RM::getShaderProgram({.vert = WDS::PT_vert, .frag = WDS::combineLighting_frag});//Combines diaphragm with lights
	RE::ShaderProgramPtr m_worldToLightsShader = RE::RM::getShaderProgram({.vert = WDS::PT_vert, .frag = WDS::worldToLight_frag});//Processes world texture to light

	RE::ShaderProgramPtr m_addDynamicLightShader = RE::RM::getShaderProgram({.vert = WDS::addDynamicLight_vert, .frag = WDS::addDynamicLight_frag});


	RE::VertexArray m_arrayPOUV;
	RE::VertexArray m_arrayLights;

	RE::Buffer<ARRAY, IMMUTABLE> m_bufferPOUV{sizeof(VertexPOUV) * 12, DYNAMIC_STORAGE};
	RE::Buffer<ARRAY, MUTABLE> m_bufferDynamicLights{STREAM, DRAW};

	//Lighting global
	float m_dayLength = 200.0f;
	float m_currentTime = 50.0f;
	glm::vec4 m_backgroundColour = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

	//Dynamic lights
	std::vector<DynamicLight> m_dynamicLights;
};