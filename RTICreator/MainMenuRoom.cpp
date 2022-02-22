#include "MainMenuRoom.hpp"

#include <locale>
#include <codecvt>

#include <Windows.h>

#include <RealEngine/external/lodepng/lodepng.hpp>
#include <RealEngine/graphics/GeometryBatch.hpp>
#include <RealEngine/main/Error.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

struct RTI {
	int content;
};

MainMenuRoom::MainMenuRoom(RE::CommandLineArguments args) :
	m_texView(window()->getDims()) {
	synchronizer()->setFramesPerSecondLimit(150u);
	glClearColor(140.0f / 255.0f, 140.0f / 255.0f, 140.0f / 255.0f, 1.0f);

	//Set last visited location to location of this executable
	m_lastVisitedLoc = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(args[0]);

	m_windowDims = window()->getDims();
	m_texView.setPosition(m_windowDims * 0.5f);

	//Menu init
	std::vector<void (MainMenuRoom::*)(const std::string&)> functions = {
		&MainMenuRoom::loadCallback,
		&MainMenuRoom::overlapCallback,

		&MainMenuRoom::formatCallback,
		&MainMenuRoom::minFilterCallback,
		&MainMenuRoom::magFilterCallback,
		&MainMenuRoom::wrapStyleXCallback,
		&MainMenuRoom::wrapStyleYCallback,
		&MainMenuRoom::borderColourCallback,

		&MainMenuRoom::subimagesSpritesCallback,
		&MainMenuRoom::dimsCallback,
		&MainMenuRoom::pivotCallback,

		&MainMenuRoom::saveAsCallback,
		&MainMenuRoom::saveCallback,
	};

	auto error = m_menu.loadMenuFromFile("menu/index.jsom", m_windowDims, &RE::SpriteBatch::std(), functions, RE::LanguageManager::std());
	if (error != RGUI::MenuParserError::OK) {
		RE::fatalError("Failed to load menu files");
	}

	m_menu.setState(2.0f);
	m_menu.getController<RGUI::Slider>("overlap_x")->setPortion(m_overlap.x);
	m_menu.getController<RGUI::Slider>("overlap_y")->setPortion(m_overlap.y);

	reloadScheme();

	if (args.size() > 1) {
		//We have second argument which should be the texture that should be open on start-up
		load(args[1]);
	}
}

MainMenuRoom::~MainMenuRoom() {

}

void MainMenuRoom::sessionStart(const RE::RoomTransitionParameters& params) {

}

void MainMenuRoom::sessionEnd() {

}

void MainMenuRoom::step() {
	m_cursorPos = (glm::vec2)input()->getCursorAbs();
	bool hovered = m_menu.step(m_cursorPos);
	if (input()->wasPressed(RE::Key::LMB)) {
		m_menu.onPress(m_cursorPos);
	}
	if (input()->isDown(RE::Key::LMB) && !hovered && m_texture) {
		m_texView.shiftPosition((glm::vec2{m_cursorPosPrev - m_cursorPos} / m_drawScale));
	}
	if (input()->wasReleased(RE::Key::LMB)) {
		m_menu.onRelease(m_cursorPos);
	}

	if (m_texture) {
		if (input()->wasPressed(RE::Key::UMW)) {
			m_texView.zoom({1.5f, 1.5f});
			m_drawScale *= 1.5f;
		}
		if (input()->wasPressed(RE::Key::DMW)) {
			m_texView.zoom({0.66666666f, 0.66666666f});
			m_drawScale *= 0.66666666f;
		}
	}
	m_cursorPosPrev = m_cursorPos;

	if (input()->wasReleased(RE::Key::R)) {
		resetView();
	}
}

void MainMenuRoom::render(double interpolationFactor) {
	auto mat = m_texView.getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &mat);
	//Texture
	if (m_texture) {
		RE::SpriteBatch::std().begin();
		RE::SpriteBatch::std().add(glm::vec4(m_botLeftOverlap, m_dimsOverlap), glm::vec4(-m_overlap.x, -m_overlap.y, 1.0f + m_overlap.x * 2.0f, 1.0f + 2.0f * m_overlap.y), m_texture->getID(), 0);
		RE::SpriteBatch::std().end(RE::GlyphSortType::POS_TOP);
		RE::SpriteBatch::std().draw();
	}
	//Scheme
	RE::GeometryBatch::std().draw();

	mat = RE::View::std.getViewMatrix();
	RE::UniformManager::std.setUniformBuffer("GlobalMatrices", 0u, sizeof(glm::mat4), &mat);

	//Menu
	RE::SpriteBatch::std().begin();
	RE::RM::getFont(m_title)->add(RE::SpriteBatch::std(), L"rtiCreator v2.0.0", m_windowDims * glm::vec2(0.5f, 0.95f), 0, RE::Colour{0u, 0u, 0u, 255u}, RE::HAlign::MIDDLE);
	RE::RM::getFont(m_FPS)->add(RE::SpriteBatch::std(), (L"FPS = " + std::to_wstring((int)synchronizer()->getFramesPerSecond()) + L" | lim = 150").c_str(), m_windowDims * glm::vec2(0.5f, 0.995f), 0, RE::Colour{0u, 0u, 0u, 255u}, RE::HAlign::MIDDLE, RE::VAlign::BELOW);
	m_menu.draw();
	RE::SpriteBatch::std().end(RE::GlyphSortType::POS_TOP);
	RE::SpriteBatch::std().draw();
}

void MainMenuRoom::resizeWindow(const glm::ivec2& newDims, bool isPermanent) {
	m_windowDims = newDims;
	window()->resize(m_windowDims, isPermanent);
	m_texView.resizeView(m_windowDims);
}

void MainMenuRoom::loadCallback(const std::string& name) {
	wchar_t filename[MAX_PATH];
	ZeroMemory(&filename, sizeof(filename));

	std::wstring filter;
	std::wstring title;
	std::string temp;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert_utf8_utf16;

	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	temp = LM(MMENU_TEXTURE_FILTERS);
	filter = convert_utf8_utf16.from_bytes(temp.data(), &temp.back() + 1);
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	temp = LM(MMENU_SELECT_TEXTURE);
	title = convert_utf8_utf16.from_bytes(temp.data());
	ofn.lpstrTitle = title.c_str();
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	ofn.lpstrInitialDir = m_lastVisitedLoc.c_str();
	m_lastVisitedLoc = filename;

	synchronizer()->pauseSteps();
	GetOpenFileNameW(&ofn);
	synchronizer()->resumeSteps();

	load(convert_utf8_utf16.to_bytes(filename));
}

void MainMenuRoom::overlapCallback(const std::string& name) {
	if (m_texture) {
		m_overlap = glm::vec2(m_menu.getController<RGUI::Slider>("overlap_x")->getPortion(), m_menu.getController<RGUI::Slider>("overlap_y")->getPortion());
		reloadScheme();
	}
}

void MainMenuRoom::formatCallback(const std::string& name) {
	if (m_texture) {
		/*auto newParams = m_texture->getParameters();
		newParams.
		m_texture->saveToFile(m_textureLoc, newParams);*/
	}
}

void MainMenuRoom::minFilterCallback(const std::string& name) {
	static const RE::TextureMinFilter filters[] = {
		RE::TextureMinFilter::NEAREST, RE::TextureMinFilter::LINEAR,
		RE::TextureMinFilter::NEAREST_MIPMAP_NEAREST, RE::TextureMinFilter::NEAREST_MIPMAP_LINEAR,
		RE::TextureMinFilter::NEAREST_MIPMAP_LINEAR, RE::TextureMinFilter::LINEAR_MIPMAP_LINEAR
	};
	if (m_texture) {
		RE::TextureMinFilter choice = filters[m_menu.getController<RGUI::ClosedList>("min_filter")->getChosen()];
		m_texture->setMinFilter(choice);
	}
}

void MainMenuRoom::magFilterCallback(const std::string& name) {
	static const RE::TextureMagFilter filters[] = {
		RE::TextureMagFilter::NEAREST, RE::TextureMagFilter::LINEAR
	};
	if (m_texture) {
		RE::TextureMagFilter choice = filters[m_menu.getController<RGUI::ClosedList>("mag_filter")->getChosen()];
		m_texture->setMagFilter(choice);
	}
}

void MainMenuRoom::wrapStyleXCallback(const std::string& name) {
	static const RE::TextureWrapStyle filters[] = {
		RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_BORDER,
		RE::TextureWrapStyle::REPEAT_NORMALLY, RE::TextureWrapStyle::REPEAT_MIRRORED
	};
	if (m_texture) {
		RE::TextureWrapStyle choice = filters[m_menu.getController<RGUI::ClosedList>("wrap_style_x")->getChosen()];
		m_texture->setWrapStyleX(choice);
	}
}

void MainMenuRoom::wrapStyleYCallback(const std::string& name) {
	static const RE::TextureWrapStyle filters[] = {
		RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_BORDER,
		RE::TextureWrapStyle::REPEAT_NORMALLY, RE::TextureWrapStyle::REPEAT_MIRRORED
	};
	if (m_texture) {
		RE::TextureWrapStyle choice = filters[m_menu.getController<RGUI::ClosedList>("wrap_style_y")->getChosen()];
		m_texture->setWrapStyleY(choice);
	}
}

void MainMenuRoom::borderColourCallback(const std::string& name) {
	if (m_texture) {
		RE::Colour col;
		col.r = (GLubyte)m_menu.getController<RGUI::Slider>("border_R")->getValue();
		col.g = (GLubyte)m_menu.getController<RGUI::Slider>("border_G")->getValue();
		col.b = (GLubyte)m_menu.getController<RGUI::Slider>("border_B")->getValue();
		col.a = (GLubyte)m_menu.getController<RGUI::Slider>("border_A")->getValue();

		m_texture->setBorderColour(col);
	}
}

void MainMenuRoom::subimagesSpritesCallback(const std::string& name) {
	if (m_texture) {
		try {
			unsigned short x = (unsigned short)std::stoul(*m_menu.getController<RGUI::TextField>("n_subimages")->getTextFieldString());
			unsigned short y = (unsigned short)std::stoul(*m_menu.getController<RGUI::TextField>("n_sprites")->getTextFieldString());
			m_texture->setSubimagesSpritesCount(glm::vec2(x, y));
			reloadScheme();
		}
		catch (...) {

		}
	}
}

void MainMenuRoom::dimsCallback(const std::string& name) {
	if (m_texture) {
		try {
			m_texture->setSubimageDims(
				glm::vec2(std::stof(*m_menu.getController<RGUI::TextField>("subimage_dims_x")->getTextFieldString()),
					std::stof(*m_menu.getController<RGUI::TextField>("subimage_dims_y")->getTextFieldString())));
			reloadScheme();
		}
		catch (...) {

		}
	}
}

void MainMenuRoom::pivotCallback(const std::string& name) {
	if (m_texture) {
		try {
			if (name == "subimage_pivot_center") {
				m_texture->setPivot(m_texture->getSubimageDims() / 2.0f);
				*m_menu.getController<RGUI::TextField>("subimage_pivot_x")->getTextFieldString() = std::to_string(m_texture->getPivot().x);
				*m_menu.getController<RGUI::TextField>("subimage_pivot_y")->getTextFieldString() = std::to_string(m_texture->getPivot().y);
			}
			m_texture->setPivot(
				glm::vec2(std::stof(*m_menu.getController<RGUI::TextField>("subimage_pivot_x")->getTextFieldString()),
					std::stof(*m_menu.getController<RGUI::TextField>("subimage_pivot_y")->getTextFieldString())));
			reloadScheme();
		}
		catch (...) {

		}
	}
}

void MainMenuRoom::saveAsCallback(const std::string& name) {
	if (m_texture) {
		wchar_t filename[MAX_PATH];
		ZeroMemory(&filename, sizeof(filename));

		std::wstring filter;
		std::wstring title;
		std::string temp;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert_utf8_utf16;

		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		temp = LM(MMENU_TEXTURE_FILTERS);
		filter = convert_utf8_utf16.from_bytes(temp.data(), &temp.back() + 1);
		ofn.lpstrFilter = filter.c_str();
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		temp = LM(MMENU_SAVE_AS_EXPLORER);
		title = convert_utf8_utf16.from_bytes(temp.data());
		ofn.lpstrTitle = title.c_str();
		ofn.Flags = OFN_DONTADDTORECENT | OFN_EXPLORER;
		ofn.lpstrInitialDir = m_lastVisitedLoc.c_str();

		synchronizer()->pauseSteps();
		GetOpenFileNameW(&ofn);
		synchronizer()->resumeSteps();

		save(convert_utf8_utf16.to_bytes(filename));
	}
}

void MainMenuRoom::saveCallback(const std::string& name) {
	if (m_texture) {
		save(m_textureLoc);
	}
}

void MainMenuRoom::save(const std::string& loc) {
	if (loc.empty()) { return; }
	m_texture->saveToFile(loc);
}

void MainMenuRoom::load(const std::string& loc) {
	if (loc.empty()) { return; }
	try {
		m_texture = RE::Texture{loc};
	}
	catch (...) {
		return;
	}
	//Save texture's location
	m_textureLoc = loc;

	//Notify buttons about texture
	m_menu.getController<RGUI::ClosedList>("format_type")->choose(m_texture->getFormatType() == RE::TextureFormatType::INTEGRAL ? 1 : 0);
	m_menu.getController<RGUI::ClosedList>("format_sign")->choose(m_texture->getFormatSign() == RE::TextureFormatSign::SIGNED ? 1 : 0);

	auto minFilterIndex = [](RE::TextureMinFilter filter) {
		switch (filter) {
		case RE::TextureMinFilter::NEAREST: return 0;
		case RE::TextureMinFilter::LINEAR: return 1;
		case RE::TextureMinFilter::NEAREST_MIPMAP_NEAREST: return 2;
		case RE::TextureMinFilter::NEAREST_MIPMAP_LINEAR: return 3;
		case RE::TextureMinFilter::LINEAR_MIPMAP_NEAREST: return 4;
		case RE::TextureMinFilter::LINEAR_MIPMAP_LINEAR: return 5;
		default: return 0;
		}
	};
	m_menu.getController<RGUI::ClosedList>("min_filter")->choose(minFilterIndex(m_texture->getMinFilter()));
	m_menu.getController<RGUI::ClosedList>("mag_filter")->choose(m_texture->getMagFilter() == RE::TextureMagFilter::LINEAR ? 1 : 0);
	auto wrapStyleIndex = [](RE::TextureWrapStyle wrapStyle) {
		switch (wrapStyle) {
		case RE::TextureWrapStyle::CLAMP_TO_EDGE: return 0;
		case RE::TextureWrapStyle::CLAMP_TO_BORDER: return 1;
		case RE::TextureWrapStyle::REPEAT_NORMALLY: return 2;
		case RE::TextureWrapStyle::REPEAT_MIRRORED: return 3;
		default: return 0;
		}
	};
	m_menu.getController<RGUI::ClosedList>("wrap_style_x")->choose(wrapStyleIndex(m_texture->getWrapStyleX()));
	m_menu.getController<RGUI::ClosedList>("wrap_style_y")->choose(wrapStyleIndex(m_texture->getWrapStyleY()));
	auto borderColour = m_texture->getBorderColour();
	m_menu.getController<RGUI::Slider>("border_R")->setPortion((float)borderColour.r / 255.0f);
	m_menu.getController<RGUI::Slider>("border_G")->setPortion((float)borderColour.g / 255.0f);
	m_menu.getController<RGUI::Slider>("border_B")->setPortion((float)borderColour.b / 255.0f);
	m_menu.getController<RGUI::Slider>("border_A")->setPortion((float)borderColour.a / 255.0f);
	*m_menu.getController<RGUI::TextField>("n_subimages")->getTextFieldString() = std::to_string((int)m_texture->getSubimagesSpritesCount().x);
	*m_menu.getController<RGUI::TextField>("n_sprites")->getTextFieldString() = std::to_string((int)m_texture->getSubimagesSpritesCount().y);
	*m_menu.getController<RGUI::TextField>("subimage_dims_x")->getTextFieldString() = std::to_string((int)m_texture->getSubimageDims().x);
	*m_menu.getController<RGUI::TextField>("subimage_dims_y")->getTextFieldString() = std::to_string((int)m_texture->getSubimageDims().y);
	*m_menu.getController<RGUI::TextField>("subimage_pivot_x")->getTextFieldString() = std::to_string((int)m_texture->getPivot().x);
	*m_menu.getController<RGUI::TextField>("subimage_pivot_y")->getTextFieldString() = std::to_string((int)m_texture->getPivot().y);

	//Scheme should be reloaded
	reloadScheme();

	//Enabling save button
	m_menu.enable("save");
}

void MainMenuRoom::reloadScheme() {
	if (m_texture) {
		m_dims = m_texture->getSubimagesSpritesCount() * m_texture->getSubimageDims();
		m_botLeftOverlap = m_windowDims * 0.5f - m_dims * (glm::vec2(0.5f, 0.5f) + m_overlap);
		m_botLeft = m_windowDims * 0.5f - m_dims * 0.5f;
		m_dimsOverlap = m_dims * (glm::vec2(1.0f, 1.0f) + 2.0f * m_overlap);

		auto& gb = RE::GeometryBatch::std();
		gb.begin();
		std::vector<RE::VertexPOCO> vertices;
		glm::vec2 subimageSprite = m_texture->getSubimagesSpritesCount();
		vertices.reserve((size_t)(subimageSprite.x * subimageSprite.y) * 4u);
		RE::Colour border{0, 255u, 0u, 255u};
		glm::vec2 dims = m_texture->getSubimageDims();
		//Subimages
		for (float x = 1.0f; x < subimageSprite.x; ++x) {
			glm::vec2 coord = m_botLeft + glm::vec2(x, 0.0f) * dims;
			vertices.emplace_back(coord, border);
			vertices.emplace_back(coord + glm::vec2(0.0f, m_dims.y), border);
		}
		for (float y = 1.0f; y < subimageSprite.y; ++y) {
			glm::vec2 coord = m_botLeft + glm::vec2(0.0f, y) * dims;
			vertices.emplace_back(coord, border);
			vertices.emplace_back(coord + glm::vec2(m_dims.x, 0.0f), border);
		}
		if (vertices.size() > 0u) {
			gb.addPrimitives(RE::PRIM::LINES, 0u, vertices.size(), vertices.data(), false);
			vertices.clear();
		}
		//Origins
		border = {0u, 0u, 255u, 255u};
		glm::vec2 origin = m_texture->getPivot();
		glm::vec2 off = glm::vec2((dims.x + dims.y) * 0.05f);
		for (float x = 0.0f; x < m_texture->getSubimagesSpritesCount().x; ++x) {
			for (float y = 0.0f; y < (size_t)m_texture->getSubimagesSpritesCount().y; ++y) {
				glm::vec2 coord = m_botLeft + glm::vec2(x, y) * dims + origin;
				vertices.emplace_back(coord + glm::vec2(off.x, off.y), border);
				vertices.emplace_back(coord + glm::vec2(-off.x, -off.y), border);
				vertices.emplace_back(coord + glm::vec2(off.x, -off.y), border);
				vertices.emplace_back(coord + glm::vec2(-off.x, off.y), border);
			}
		}
		gb.addPrimitives(RE::PRIM::LINES, 0u, vertices.size(), vertices.data(), false);
		vertices.clear();
		//Whole image
		border = {255u, 0u, 0u, 255u};
		vertices.emplace_back(m_botLeft, border);
		vertices.emplace_back(m_botLeft + glm::vec2(m_dims.x, 0.0f), border);
		vertices.emplace_back(m_botLeft + glm::vec2(m_dims.x, m_dims.y), border);
		vertices.emplace_back(m_botLeft + glm::vec2(0.0f, m_dims.y), border);
		gb.addPrimitives(RE::PRIM::LINE_LOOP, 0u, vertices.size(), vertices.data());

		gb.end();
		gb.end();
	}
}

void MainMenuRoom::resetView() {
	m_texView.setScale({1.0f, 1.0f});
	m_texView.setPosition(m_windowDims * 0.5f);
	m_drawScale = 1.0f;
	reloadScheme();
}
