#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/main/Room.hpp>
#define RE_MENU_KEEP_INFOS
#include <RealEngine/GUI/Menu.hpp>

/**
 * @brief Is the room that holds the main menu and related objects.
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
	RGUI::Menu<MainMenuRoom> m_menu{program(), this};
	std::vector<std::string> m_savesButtons;

	void buildSavesButtons();

	void mainMenuCallback(const std::string& button);
	void newWorldCallback(const std::string& button);
	void loadWorldCallback(const std::string& button);
	void deleteWorldCallback(const std::string& button);

	void loadWorld(const std::string& worldName);
};