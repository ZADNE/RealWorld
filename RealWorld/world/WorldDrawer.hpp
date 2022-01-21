#pragma once
#include <map>

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/shaders/WDS.hpp>
#include <RealWorld/world/StaticLight.hpp>
#include <RealWorld/world/DynamicLight.hpp>
#include <RealWorld/world/DestroyTicket.hpp>
#include <RealWorld/world/LightManipulator.hpp>

class ChunkHandler;

class WorldDrawer {
	friend class LightManipulator;
public:
	WorldDrawer();
	~WorldDrawer();

	void init(const glm::uvec2& viewSizePx, const glm::mat4& viewMatrix, ChunkHandler* chunkHandler);
	void setTarget(const glm::ivec2& worldDimBc, GLuint worldTexture);
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
	//Returns ticket used for removing the light
	DestroyTicket<StaticLight> staticLight_add(const StaticLight& staticLight);
	//Ticket is obtained from addStaticLight()
	//Invalid ticket are silently ingnored
	void staticLight_remove(DestroyTicket<StaticLight> ticket);
	//Reserve space for future lights
	//Does not ever have to be called, use before adding large amount of lights
	//add = amount of lightspaces to add to the current number
	//Cannot remove or alter resident lights
	void staticLight_reserve(size_t add);
	//Unlike static lights, dynamic lights must be added each step between beginStep() and endStep()
	void dynamicLight_add(const DynamicLight& dynamicLight);
	void dynamicLight_add(const glm::vec2& posPx, RE::Colour col, GLfloat dir, GLfloat cone);



	void reloadViewSize(const glm::uvec2& viewSizePx);

	void generateMesh();
	void createGLObjects();

	void generateMeshLighting();
	void createGLObjectsLighting();

	void createGLObjectsStaticLights();
	void createGLObjectsDynamicLights();

	void initShaders(const glm::mat4& viewMatrix);
	void updateUniformsAfterResize(const glm::mat4& newViewMatrix);

	RE::Surface m_SurLighting{{RE::TextureFlags::RGBA_NU_NEAR_NEAR_EDGE}, false, false};
	//0 texture (diaphragm):			R = diaphragm of the tile; G = direction of the light; B = width of light cone; A = unused yet
	//1 texture (lighting):				RGB = colour of the light; A = strength of the light
	//2 texture (finished lighting):	RGBA = drawn without change to screen

	glm::vec2 m_botLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftBc;//Bottom-left corner of the view in blocks
	glm::vec2 m_viewsizePx;
	glm::vec2 m_viewsizePxLarger;//m_viewsizeBc * BLOCK_SIZE; larger than m_viewsizePx
	glm::uvec2 m_viewsizeBc;
	glm::uvec2 m_viewsizeLightingBc;
	ChunkHandler* m_chunkHandler;

	glm::ivec2 m_worldDimBc;

	RE::TexturePtr m_blockAtlasTex;
	RE::TexturePtr m_wallAtlasTex;

	RE::ShaderProgramPtr m_standardDraw = RE::RM::getShaderProgram({.vert = WDS::tilesDraw_vert, .frag = WDS::tilesDraw_frag });
	RE::ShaderProgramPtr m_finalLighting = RE::RM::getShaderProgram({.vert = WDS::finalLighting_vert, .frag = shaders::gauss32_frag });//Used to draw finished lighting to the screen

	RE::ShaderProgramPtr m_combineLighting = RE::RM::getShaderProgram({.vert = WDS::PT_vert, .frag = WDS::combineLighting_frag });//Combine finished diaphragm with lighting to produce finished lighting
	RE::ShaderProgramPtr m_worldToLight = RE::RM::getShaderProgram({.vert = WDS::PT_vert, .frag = WDS::worldToLight_frag });//Process world texture to light

	RE::ShaderProgramPtr m_sumSL = RE::RM::getShaderProgram({.vert = WDS::addStaticLight_vert, .frag = WDS::sumStaticLight_frag });
	RE::ShaderProgramPtr m_addSL = RE::RM::getShaderProgram({.vert = WDS::addStaticLight_vert, .frag = WDS::addStaticLight_frag });
	RE::ShaderProgramPtr m_addDL = RE::RM::getShaderProgram({.vert = WDS::addDynamicLight_vert, .frag = WDS::addDynamicLight_frag });

	//Tile drawing
	GLuint m_VAO = 0u;
	GLuint m_VBO = 0u;

	//Lighting global
	GLuint m_VAOLighting = 0u;
	GLuint m_VBOLighting = 0u;//0 - 5 block, 6 - 11 pixel
	float m_dayLength = 200.0f;
	float m_currentTime = 50.0f;
	glm::vec4 m_backgroundColour = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

	//Static lights
	GLuint m_VAO_SLs = 0u;
	GLuint m_VBO_SLs = 0u;
	std::vector<StaticLight> m_staticLights;
	std::map<DestroyTicket<StaticLight>, size_t> m_destroyIndexes;
	size_t m_nextTicket = 0u;
	void staticLight_reserveInternal(size_t newCapacity);

	//Dynamic lights
	GLuint m_VAO_DLs = 0u;
	GLuint m_VBO_DLs = 0u;
	size_t m_VBO_DLs_size = 0u;
	std::vector<DynamicLight> m_dynamicLights;
};