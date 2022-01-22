#include <RealWorld/menu/MainMenuRoom.hpp>

#include <RealEngine/graphics/View.hpp>

#include <RealWorld/world/WorldCreator.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>


MainMenuRoom::MainMenuRoom(RE::CommandLineArguments args) {
	//Menu init
	std::vector<void (MainMenuRoom::*)(const std::string&)> functions = {
		&MainMenuRoom::mainMenuCallback,
		&MainMenuRoom::newWorldCallback,
		&MainMenuRoom::loadWorldCallback,
		&MainMenuRoom::deleteWorldCallback
	};
	auto error = m_menu.loadMenuFromFile("menu/index.jsom", window()->getDims(), &RE::SpriteBatch::std(), functions);
	if (error != RGUI::MenuParserError::OK) {
		RE::fatalError("Failed to load menu files");
	}
}


MainMenuRoom::~MainMenuRoom() {

}

void MainMenuRoom::E_entry(RE::RoomTransitionParameters params) {
	synchronizer()->setFramesPerSecondLimit(50u);
	//Background color
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
}

RE::RoomTransitionParameters MainMenuRoom::E_exit() {
	return {&m_worldToLoad};
}

void MainMenuRoom::E_step() {
	auto cursorPos = (glm::vec2)input()->getCursorAbs();

	m_menu.step(cursorPos);
	if (input()->wasPressed(RE::Key::LMB)) {
		m_menu.onPress(cursorPos);
	}
	if (input()->wasReleased(RE::Key::LMB)) {
		m_menu.onRelease(cursorPos);
	}
	if (input()->wasReleased(RE::Key::Escape)) {
		m_menu.setState(1.0f);
	}
}

void MainMenuRoom::E_draw(double interpolationFactor) {
	RE::SpriteBatch::std().begin(RE::GlyphSortType::POS_TOP);
	m_menu.draw();
	RE::SpriteBatch::std().end();
	RE::SpriteBatch::std().draw();
}

void MainMenuRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	window()->resize(newDims, isPermanent);
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
		glm::vec2 windowDims = window()->getDims();
		glm::vec2 pos = mainInfo.pos.get(); pos.y -= 0.05f * windowDims.y;
		mainInfo.pos.set(pos);
		pos = delInfo.pos.get(); pos.y -= 0.05f * windowDims.y;
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
		program()->scheduleProgramExit();
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
	m_worldToLoad = worldName;
	program()->scheduleNextRoom(1);
}
