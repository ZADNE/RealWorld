#pragma once
#include <memory>

#include <GL/glew.h>

#include <RealEngine/MainProgram.hpp>

#include <RealWorld/world/WorldRoom.hpp>
#include <RealWorld/menu/MainMenuRoom.hpp>


enum class GameState {
	PLAY,
	EXIT
};

class Game : public RE::MainProgram {
public:
	Game();
	~Game();

	virtual void E_Init(const std::vector<std::string>& cmdLnArgs) override;
	virtual void E_Exit() override;
private:
	std::unique_ptr<WorldRoom> m_worldRoom = nullptr;
	std::unique_ptr<MainMenuRoom> m_mainMenuRoom = nullptr;
};