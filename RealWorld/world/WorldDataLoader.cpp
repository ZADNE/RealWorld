﻿#include <RealWorld/world/WorldDataLoader.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>

#include <SDL2/SDL_timer.h>
#include <nlohmann/json.hpp>

#include <RealEngine/external/lodepng/lodepng.hpp>
#include <RealEngine/main/Error.hpp>

template<class T>
void writeBinary(std::ofstream& file, const T& x) {
	file.write(reinterpret_cast<const char*>(&x), sizeof(x));
}

template<class T>
void readBinary(std::ifstream& file, T& x) {
	file.read(reinterpret_cast<char*>(&x), sizeof(x));
}

std::string WorldDataLoader::m_saveFolder = "saves";

const std::string WORLD_INFO_FILENAME = "world_info.json";

bool WorldDataLoader::loadWorldData(WorldData& data, const std::string& worldName) {
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + worldName + "/";
	data.path = pathToFolder;

	try {
		loadInfo(data.wi, pathToFolder);
		loadPlayer(data.pd, pathToFolder);
	}
	catch (...) {
		return false;
	}

	std::cout << "Successfully loaded world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldDataLoader::saveWorldData(const WorldData& data, const std::string& worldName, bool creatingNew) {
	if (worldName == "") return false;
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + worldName + "/";
	bool alreadyExists = std::filesystem::exists(pathToFolder);
	if (alreadyExists && creatingNew) return false;

	if (!alreadyExists) {
		std::filesystem::create_directory(pathToFolder);
	}

	try {
		saveInfo(data.wi, pathToFolder);
		savePlayer(data.pd, pathToFolder);
	}
	catch (...) {
		return false;
	}

	std::cout << "Successfully saved world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldDataLoader::deleteWorld(const std::string& worldName) {
	if (std::filesystem::remove_all(m_saveFolder + "/" + worldName) > 0) {
		return true;
	} else {
		return false;
	}
}

void WorldDataLoader::getSavedWorlds(std::vector<std::string>& names) {
	names.clear();

	for (const auto& entry : std::filesystem::directory_iterator(m_saveFolder)) {
		names.push_back(entry.path().filename().string());
	}
}

void WorldDataLoader::loadInfo(WorldInfo& wi, const std::string& path) {
	std::ifstream i(path + WORLD_INFO_FILENAME);
	nlohmann::json j;
	i >> j;
	wi.worldName = j["world"]["name"].get<std::string>();
	wi.seed = j["world"]["seed"].get<int>();
}

void WorldDataLoader::loadPlayer(PlayerData& pd, const std::string& path) {
	std::ifstream stream(path + "player_state.rst", std::ios::binary);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}

	stream.seekg(0, std::ios::end);
	auto fileSize = stream.tellg();
	stream.seekg(0, std::ios::beg);

	if (fileSize < sizeof(pd.pos)) throw std::exception{};

	readBinary(stream, pd.pos);

	loadInventoryData(pd.id, path);
}

void WorldDataLoader::loadInventoryData(InventoryData& id, const std::string& path) {
	std::ifstream stream(path + "player_inventory.rin", std::ios::binary);
	if (stream.fail()) {
		perror((path + "player_inventory.rin").c_str());
		throw std::exception{};
	}
	glm::ivec2 newSize;

	stream.seekg(0, std::ios::end);
	auto fileSize = stream.tellg();
	stream.seekg(0, std::ios::beg);

	if ((size_t)fileSize < sizeof(newSize)) throw std::exception{};
	readBinary(stream, newSize);

	if ((size_t)fileSize < ((size_t)newSize.x * (size_t)newSize.y * sizeof(Item) + sizeof(newSize))) throw std::exception{};

	//All checks passed, loading inventory
	id.resize(newSize);

	for (int i = 0; i < id.slotCount(); ++i) {
		readBinary(stream, id(i));
	}
}

void WorldDataLoader::saveInfo(const WorldInfo& wi, const std::string& path) {
	nlohmann::json j = {
		{"world", {
			{"name", wi.worldName},
			{"seed", wi.seed}
		}}
	};

	std::ofstream o(path + WORLD_INFO_FILENAME, std::ofstream::trunc);
	o << j.dump(2);
	o.close();
}

void WorldDataLoader::savePlayer(const PlayerData& pd, const std::string& path) {
	std::ofstream stream(path + "player_state.rst", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}
	writeBinary(stream, pd.pos);

	saveInventoryData(pd.id, path);
}

void WorldDataLoader::saveInventoryData(const InventoryData& id, const std::string& path) {
	std::ofstream stream(path + "player_inventory.rin", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror(path.c_str());
		throw std::exception{};
	}

	writeBinary(stream, id.getSize());

	for (int i = 0; i < id.slotCount(); ++i) {
		writeBinary(stream, id(i));
	}
}
