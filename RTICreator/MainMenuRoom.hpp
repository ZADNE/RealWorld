#pragma once
#include <optional>

#include <RealEngine/main/Room.hpp>
#include "LanguageManager.hpp"
//#define MENU_KEEP_INFOS
#include <RealEngine/GUI/Menu.hpp>
#include <RealEngine/graphics/View.hpp>
#include <RealEngine/graphics/Font.hpp>

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

/**
 * @brief Room with the UI
*/
class MainMenuRoom : public RE::Room {
public:
	MainMenuRoom(RE::CommandLineArguments args);
	~MainMenuRoom();

	virtual void sessionStart(const RE::RoomTransitionParameters& params) override;
	virtual void sessionEnd() override;
	virtual void step() override;
	virtual void render(double interpolationFactor) override;

	void resizeWindow(const glm::ivec2& newDims, bool isPermanent);

private:
	void loadCallback(const std::string& name);
	void overlapCallback(const std::string& name);

	void formatCallback(const std::string& name);
	void minFilterCallback(const std::string& name);
	void magFilterCallback(const std::string& name);
	void wrapStyleXCallback(const std::string& name);
	void wrapStyleYCallback(const std::string& name);
	void borderColourCallback(const std::string& name);

	void subimagesSpritesCallback(const std::string& name);
	void dimsCallback(const std::string& name);
	void pivotCallback(const std::string& name);

	void saveAsCallback(const std::string& name);
	void saveCallback(const std::string& name);

	void save(const std::string& loc);
	void load(const std::string& filePath);

	//Texture
	std::optional<RE::Texture> m_texture;
	std::string m_textureLoc;
	std::wstring m_lastVisitedLoc;

	RGUI::Menu<MainMenuRoom> m_menu{program(), this};
	RE::FontSeed m_title{"arial", 38};
	RE::FontSeed m_FPS{"arial", 16};
	glm::vec2 m_windowDims = glm::vec2(0.0f, 0.0f);

	glm::vec2 m_cursorPos = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_cursorPosPrev = glm::vec2(0.0f, 0.0f);

	//Drawing
	RE::View m_texView;
	glm::vec2 m_overlap = glm::vec2(0.2f, 0.2f);
	glm::vec2 m_botLeftOverlap = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_botLeft = glm::vec2(0.0f, 0.0f);

	glm::vec2 m_dimsOverlap = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_dims = glm::vec2(0.0f, 0.0f);


	glm::vec2 m_offset = glm::vec2(0.0f, 0.0f);
	void reloadScheme();
	void resetView();

	float m_drawScale = 1.0f;
};