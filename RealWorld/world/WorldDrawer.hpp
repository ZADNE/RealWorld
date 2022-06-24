/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/world/TileDrawer.hpp>
#include <RealWorld/world/ShadowDrawer.hpp>
#include <RealWorld/world/MinimapDrawer.hpp>

 /**
  * @brief Renders the world (i.e. tiles, shadows, minimap)
 */
class WorldDrawer {
public:
	WorldDrawer(const glm::uvec2& viewSizePx);
	~WorldDrawer();

	void setTarget(const glm::ivec2& worldDimTi);
	void resizeView(const glm::uvec2& viewSizePx);

	struct ViewEnvelope {
		glm::vec2 botLeftTi;
		glm::vec2 topRightTi;
	};
	ViewEnvelope setPosition(const glm::vec2& botLeftPx);


	/**
	 * @brief Lights have to be added between beginStep() and endStep()
	*/
	void beginStep();

	/**
	 * @brief Lights have to be added between beginStep() and endStep()
	*/
	void endStep();

	void drawTiles();

	void shouldDrawShadows(bool should) { m_drawShadows = should; }
	void drawShadows();

	void shouldDrawMinimap(bool should) { m_drawMinimap = should; }
	void drawMinimap();
private:
	void updateUniformsAfterViewResize();

	glm::vec2 m_botLeftPx;//Bottom-left corner of the view
	glm::ivec2 m_botLeftTi;//Bottom-left corner of the view in tiles

	glm::vec2 m_viewSizePx;
	glm::uvec2 m_viewSizeTi;
	glm::uvec2 viewSizeTi(const glm::vec2& viewSizePx) const;

	bool m_drawShadows = true;
	bool m_drawMinimap = false;

	struct WorldDrawerUniforms {
		glm::mat4 viewsizePxMat;
		int viewWidthTi;
	};
	RE::TypedBuffer m_uniformBuf{UNIF_BUF_WORLDDRAWER, sizeof(WorldDrawerUniforms), RE::BufferUsageFlags::DYNAMIC_STORAGE};

	RE::VertexArray m_vao;//Attribute-less vertex array

	TileDrawer m_tileDrawer;
	ShadowDrawer m_shadowDrawer;
	MinimapDrawer m_minimapDrawer;
};
