#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/main/Room.hpp>

#define MENU_KEEP_INFOS
#include <RealEngine/GUI/Menu.hpp>

class MainMenuRoom : public RE::Room {
public:
	MainMenuRoom(RE::CommandLineArguments args);
	~MainMenuRoom();
	virtual void E_entry(RE::RoomTransitionParameters params) override;
	virtual RE::RoomTransitionParameters E_exit() override;
	virtual void E_step() override;
	virtual void E_draw(double interpolationFactor) override;

	void resizeWindow(const glm::ivec2& newDims, bool isPermanent);

private:
	RGUI::Menu<MainMenuRoom> m_menu{program(), this};
	std::vector<std::string> m_savesButtons;

	std::string m_worldToLoad;

	void buildSavesButtons();

	void mainMenuCallback(const std::string& button);
	void newWorldCallback(const std::string& button);
	void loadWorldCallback(const std::string& button);
	void deleteWorldCallback(const std::string& button);

	void loadWorld(const std::string& worldName);
};