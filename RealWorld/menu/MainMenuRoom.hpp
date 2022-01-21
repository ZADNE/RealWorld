#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/main/Room.hpp>

#define MENU_KEEP_INFOS
#include <RealEngine/GUI/Menu.hpp>

class MainMenuRoom : public RE::Room {
public:
	MainMenuRoom();
	~MainMenuRoom();

	virtual void E_build(const std::vector<std::string>& buildArgs) override;
	virtual void E_destroy() override;
	virtual void E_entry(std::vector<void*> enterPointers) override;
	virtual std::vector<void*> E_exit() override;
	virtual void E_step() override;
	virtual void E_draw(double interpolationFactor) override;

	virtual int getNextIndex() const override;
	virtual int getPrevIndex() const override;

	void resizeWindow(const glm::ivec2& newDims, bool isPermanent);

private:
	glm::vec2 m_windowDim;

	RGUI::Menu<MainMenuRoom> m_menu{this};
	std::vector<std::string> m_savesButtons;
	void buildSavesButtons();

	void mainMenuCallback(const std::string& button);
	void newWorldCallback(const std::string& button);
	void loadWorldCallback(const std::string& button);
	void deleteWorldCallback(const std::string& button);

	void loadWorld(const std::string& worldName);
};