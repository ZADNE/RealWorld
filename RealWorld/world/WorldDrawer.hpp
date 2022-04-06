﻿#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/world_drawing.hpp>
#include <RealWorld/world/DynamicLight.hpp>
#include <RealWorld/world/LightManipulator.hpp>
#include <RealWorld/rendering/Vertex.hpp>
#include <RealWorld/rendering/UniformBuffers.hpp>


class WorldDrawer {
	friend class LightManipulator;
public:
	WorldDrawer(const glm::uvec2& viewSizePx);
	~WorldDrawer();

	void setTarget(const glm::ivec2& worldDimTi);
	void resizeView(const glm::uvec2& newViewSizePx);

	//For adding and removing lights
	LightManipulator getLightManipulator();

	struct ViewEnvelope {
		glm::vec2 botLeftTi;
		glm::vec2 topRightTi;
	};
	ViewEnvelope setPosition(const glm::vec2& botLeftPx);

	//All dynamic lights must be added each between beginStep() and endStep()
	void beginStep();
	//All dynamic lights must be added each between beginStep() and endStep()
	void endStep();

	//Should be called at the beginning of draw beginStep
	void drawTiles();

	void toggleDarkness() { m_drawDarkness = !m_drawDarkness; }
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

	RE::Surface m_surLighting{{RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}, false, false};
	//0 texture (diaphragm):			R = diaphragm of the tile; G = direction of the light; B = width of light cone; A = unused yet
	//1 texture (lights):				RGB = colour of the light; A = strength of the light
	//2 texture (finished lighting):	RGBA = drawn to screen

	glm::vec2 m_invBotLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftTi;//Bottom-left corner of the view in blocks

	glm::vec2 m_viewDimsPx;
	glm::uvec2 m_viewDimsTi;
	glm::uvec2 m_viewDimsUn;//Units that overlap the view

	glm::ivec2 m_worldDimTi;

	RE::TexturePtr m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	RE::TexturePtr m_wallAtlasTex = RE::RM::getTexture("wallAtlas");

	RE::ShaderProgram m_tilesShader = RE::ShaderProgram({.vert = tilesDraw_vert, .frag = colorDraw_frag});
	RE::ShaderProgram m_coverWithDarknessShader = RE::ShaderProgram({.vert = coverWithDarkness_vert, .frag = colorDraw_frag});
	RE::ShaderProgram m_computeLightingShader = RE::ShaderProgram({.vert = PT_vert, .frag = computeLighting_frag});//Combines diaphragm with lights
	RE::ShaderProgram m_worldToLightsShader = RE::ShaderProgram({.vert = PT_vert, .frag = worldToLight_frag});//Processes world texture to light

	RE::ShaderProgram m_addDynamicLightShader = RE::ShaderProgram({.vert = addDynamicLight_vert, .frag = addDynamicLight_frag});

	RE::ShaderProgram m_minimapShader = RE::ShaderProgram{{.vert = minimap_vert, .frag = minimap_frag}};
	bool m_drawDarkness = true;
	bool m_drawMinimap = false;


	RE::VertexArray m_arrayPOUV;
	RE::VertexArray m_arrayLights;

	RE::Buffer<ARRAY, IMMUTABLE> m_bufferPOUV{sizeof(VertexPOUV) * 16, DYNAMIC_STORAGE};
	RE::Buffer<ARRAY, MUTABLE> m_bufferDynamicLights{STREAM, DRAW};

	struct WorldDrawUniforms {
		glm::mat4 viewsizePxMat;
		glm::mat4 viewsizeLightingUnMat;
	};
	RE::UniformBuffer m_worldDrawUniformBuffer{UNIF_BUF_WORLDDRAWER, true, sizeof(WorldDrawUniforms), DYNAMIC_STORAGE};

	//Lighting global
	float m_dayLength = 200.0f;
	float m_currentTime = 50.0f;

	//Dynamic lights
	std::vector<DynamicLight> m_dynamicLights;
};