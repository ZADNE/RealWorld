#include <RealWorld/Game.hpp>

Game::Game(){

}

Game::~Game(){

}

void Game::E_Init(const std::vector<std::string>& cmdLnArgs){
	setWindowTitle("Realworld");

	m_mainMenuRoom = std::make_unique<MainMenuRoom>();
	m_worldRoom = std::make_unique<WorldRoom>();
	p_roomVector->addRoom(m_mainMenuRoom.get(), cmdLnArgs);
	p_roomVector->addRoom(m_worldRoom.get(), cmdLnArgs);

	p_roomVector->gotoRoom(m_mainMenuRoom->getMyIndex(), {});
}

void Game::E_Exit(){

}