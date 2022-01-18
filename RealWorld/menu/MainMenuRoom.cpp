#include <RealWorld/menu/MainMenuRoom.hpp>

#include <RealEngine/View.hpp>

#include <RealWorld/world/WorldCreator.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>


MainMenuRoom::MainMenuRoom() {

}


MainMenuRoom::~MainMenuRoom() {

}

void MainMenuRoom::E_build(const std::vector<std::string>& buildArgs) {
	m_windowDim = p_MP->getWindowDim();
	//Menu init
	std::vector<void (MainMenuRoom::*)(const std::string&)> functions = {
		&MainMenuRoom::mainMenuCallback,
		&MainMenuRoom::newWorldCallback,
		&MainMenuRoom::loadWorldCallback,
		&MainMenuRoom::deleteWorldCallback
	};
	auto error = m_menu.loadMenuFromFile("menu/index.jsom", m_windowDim, &RE::SpriteBatch::std(), functions, RE::TypingHandle{p_MP});
	if (error != RGUI::MenuParserError::OK) {
		RE::fatalError("Failed to load menu files");
	}
}

void MainMenuRoom::E_destroy() {

}

void MainMenuRoom::E_entry(std::vector<void*> enterPointers) {
	p_MP->setFramesPerSecondLimit(50u);
	//Background color
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
}

std::vector<void*> MainMenuRoom::E_exit() {
	return p_leavePointers;
}

void MainMenuRoom::E_step() {
	RE::InputManager* IM = p_MP->IM();
	auto cursorPos = (glm::vec2)IM->getCursorAbs();

	m_menu.step(cursorPos);
	if (IM->wasPressed(RE::RKey::LMB)) {
		m_menu.onPress(cursorPos);
	}
	if (IM->wasReleased(RE::RKey::LMB)) {
		m_menu.onRelease(cursorPos);
	}
	if (IM->wasReleased(RE::RKey::Escape)) {
		m_menu.setState(1.0f);
	}
}

void MainMenuRoom::E_draw(double interpolationFactor) {
	RE::SpriteBatch::std().begin(RE::GlyphSortType::POS_TOP);
	m_menu.draw();
	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
}

int MainMenuRoom::getNextIndex() const {
	return 1;
}

int MainMenuRoom::getPrevIndex() const {
	return Room::NO_ROOM_INDEX;
}

void MainMenuRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	p_MP->resizeWindow(newDims, isPermanent);
	m_windowDim = newDims;
}

void MainMenuRoom::buildSavesButtons() {
	for (auto& name : m_savesButtons) {
		m_menu.remove(name);
		m_menu.remove("del_" + name);
	}
	m_savesButtons.clear();

	std::vector<std::string> names;
	WorldDataLoader::getSavedWorlds(names);
	auto mainInfo = m_menu.getInfo("load_world_template");
	mainInfo.type.set(RGUI::ControllerType::BUTTON);
	auto delInfo = m_menu.getInfo("delete_world_template");
	delInfo.type.set(RGUI::ControllerType::BUTTON);
	for (auto& name : names) {
		m_savesButtons.push_back(name);
		mainInfo.text.set(name);
		m_menu.addController(name, mainInfo);
		m_menu.addController("del_" + name, delInfo);
		glm::vec2 pos = mainInfo.pos.get(); pos.y -= 0.05f * m_windowDim.y;
		mainInfo.pos.set(pos);
		pos = delInfo.pos.get(); pos.y -= 0.05f * m_windowDim.y;
		delInfo.pos.set(pos);
	}
}

void MainMenuRoom::mainMenuCallback(const std::string& button) {
	if (button == "new_world") {
		m_menu.setState(2.0f);
		*m_menu.getController<RGUI::TextField>("name")->getTextFieldString() = "";
		*m_menu.getController<RGUI::TextField>("seed")->getTextFieldString() = "";
		*m_menu.getController<RGUI::TextField>("chunk_width")->getTextFieldString() = "";
		*m_menu.getController<RGUI::TextField>("chunk_height")->getTextFieldString() = "";
	} else if (button == "load_world") {
		buildSavesButtons();
		m_menu.setState(3.0f);
	} else if (button == "exit") {
		p_MP->exitProgram();
	} else if (button == "return") {
		m_menu.setState(1.0f);
	}
}

void MainMenuRoom::newWorldCallback(const std::string& button) {
	try {
		auto name = *m_menu.getController<RGUI::TextField>("name")->getTextFieldString();
		if (name == "") throw std::exception{};
		int seed = std::stoi(*m_menu.getController<RGUI::TextField>("seed")->getTextFieldString());
		unsigned width = 128;
		unsigned height = 128;
		try {
			width = std::stoul(*m_menu.getController<RGUI::TextField>("chunk_width")->getTextFieldString());
		}
		catch (...) {}
		try {
			height = std::stoul(*m_menu.getController<RGUI::TextField>("chunk_height")->getTextFieldString());
		}
		catch (...) {}


		WorldCreator creator;
		auto wd = creator.createWorld(name, seed);
		WorldDataLoader::saveWorldData(wd, name);
		m_menu.setState(1.0f);
		loadWorld(name);
	}
	catch (...) {
		return;
	}
}

void MainMenuRoom::loadWorldCallback(const std::string& button) {
	loadWorld(button);
}

void MainMenuRoom::deleteWorldCallback(const std::string& button) {
	std::string world = button.substr(4);
	if (WorldDataLoader::deleteWorld(world)) {
		buildSavesButtons();
	}
}

void MainMenuRoom::loadWorld(const std::string& worldName) {
	p_leavePointers.clear();
	p_leavePointers.push_back(new std::string(worldName));
	p_MP->goToRoom(1);
}
