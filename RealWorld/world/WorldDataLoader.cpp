#include "WorldDataLoader.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>

#include <SDL/SDL_timer.h>
#include <nlohmann/json.hpp>

#include <RealEngine/lodepng.hpp>
#include <RealEngine/Error.hpp>

#include "../furniture/FDB.hpp"

#define writeBin(x) write(reinterpret_cast<const char *>(&##x##), sizeof(##x##))
#define readBin(x) read(reinterpret_cast<char *>(&##x##), sizeof(##x##))

std::string WorldDataLoader::m_saveFolder = "saves";

const std::string WORLD_INFO_FILENAME = "world_info.json";

bool WorldDataLoader::loadWorldData(WorldData& data, const std::string& worldName) {
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + worldName + "/";
	data.path = pathToFolder;
	
	try {
		loadInfo(data.wi, pathToFolder);
		loadPlayer(data.pd, pathToFolder);
		loadFurniture(data.fc, pathToFolder);
	} catch (...) {
		return false;
	}

	std::cout << "Successfully loaded world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldDataLoader::saveWorldData(const WorldData& data, const std::string& worldName) {
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + worldName + "/";
	if (!std::filesystem::exists(pathToFolder)) {
		std::filesystem::create_directory(pathToFolder);
	}

	try {
		saveInfo(data.wi, pathToFolder);
		savePlayer(data.pd, pathToFolder);
		saveFurniture(data.fc, pathToFolder);
	} catch (...) {
		return false;
	}

	std::cout << "Successfully saved world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldDataLoader::deleteWorld(const std::string& worldName){
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

void WorldDataLoader::loadInfo(WorldInfo& wi, const std::string& path){
	std::ifstream i(path + WORLD_INFO_FILENAME);
	nlohmann::json j;
	i >> j;
	wi.worldName = j["world"]["name"].get<std::string>();
	wi.seed = j["world"]["seed"].get<int>();
	auto vec = j["world"]["chunk_dims"].get<std::vector<unsigned int>>();
	if (vec.size() != 2) throw std::exception();
	wi.chunkDims.x = vec[0]; wi.chunkDims.y = vec[1];
	if (wi.chunkDims.x <= 0 || wi.chunkDims.y <= 0) throw std::exception();
}

void WorldDataLoader::loadPlayer(PlayerData& pd, const std::string& path){
	std::ifstream stream(path + "player_state.rst", std::ios::binary);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}

	stream.seekg(0, std::ios::end);
	auto fileSize = stream.tellg();
	stream.seekg(0, std::ios::beg);

	if (fileSize < sizeof(pd.pos)) throw std::exception{};

	stream.readBin(pd.pos);

	loadInventoryData(pd.id, path);
}

void WorldDataLoader::loadInventoryData(InventoryData& id, const std::string& path){
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
	stream.readBin(newSize);

	if ((size_t)fileSize < ((size_t)newSize.x * (size_t)newSize.y * sizeof(Item) + sizeof(newSize))) throw std::exception{};

	//All checks passed, loading inventory
	id.resize(newSize);

	for (int x = 0; x < id.size.x; x++) {
		for (int y = 0; y < id.size.y; y++) {
			Item& item = id.items[x][y];
			stream.readBin(item);
		}
	}
}

void WorldDataLoader::loadFurniture(FurnitureCollection& fc, const std::string& path){
	std::ifstream stream(path + "furniture_register.rfr", std::ios::binary);
	if (stream.fail()) {
		perror((path + "furniture_register.rfr").c_str());
		throw std::exception{};
	}

	const ulong entrySize = sizeof(glm::ivec2) + sizeof(ulong);

	stream.seekg(0, std::ios::end);
	auto streamSize = stream.tellg();
	stream.seekg(0, std::ios::beg);
	streamSize -= stream.tellg();

	glm::ivec2 pos = glm::ivec2(0, 0);
	ulong totalIndex = 0;
	for (ulong i = 0u; i < streamSize; i += entrySize) {
		stream.readBin(pos);
		stream.readBin(totalIndex);
		switch (FDB::getType(totalIndex)) {
		case F_TYPE::STATIC:
			fc.f0.emplace_back(pos, totalIndex); break;
		case F_TYPE::CHEST:
			fc.f1.emplace_back(FStatic{ pos, totalIndex }); break;
		case F_TYPE::CRAFTING_STATION:
			fc.f2.emplace_back(FStatic{ pos, totalIndex }); break;
		case F_TYPE::FURNACE:
			fc.f3.emplace_back(FStatic{ pos, totalIndex }); break;
		case F_TYPE::LIGHT_SOURCE:
			fc.f4.emplace_back(FStatic{ pos, totalIndex }); break;
		}
	}
}

void WorldDataLoader::saveInfo(const WorldInfo& wi, const std::string& path){
	nlohmann::json j = {
		{"world", {
			{"name", wi.worldName},
			{"seed", wi.seed},
			{"chunk_dims", { wi.chunkDims.x, wi.chunkDims.y }}
		}}
	};

	std::ofstream o(path + WORLD_INFO_FILENAME, std::ofstream::trunc);
	o << j.dump(2);
	o.close();
}

void WorldDataLoader::savePlayer(const PlayerData& pd, const std::string& path){
	std::ofstream stream(path + "player_state.rst", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}
	stream.writeBin(pd.pos);

	saveInventoryData(pd.id, path);
}

void WorldDataLoader::saveInventoryData(const InventoryData& id, const std::string& path){
	std::ofstream stream(path + "player_inventory.rin", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror(path.c_str());
		throw std::exception{};
	}

	stream.writeBin(id.size);

	for (int x = 0; x < id.size.x; x++) {
		for (int y = 0; y < id.size.y; y++) {
			Item item = id.items[x][y];
			stream.writeBin(item);
		}
	}
}

void WorldDataLoader::saveFurniture(const FurnitureCollection& fc, const std::string& path){
	std::ofstream stream(path + "furniture_register.rfr", std::ios::binary);
	if (stream.fail()) {
		perror((path + "furniture_register.rfr").c_str());
		throw std::exception{};
	}
	for (auto& fur: fc.f0) {
		glm::ivec2 pos = fur.getBotLeft();
		stream.writeBin(pos);
		ulong totalIndex = (ulong)fur.getTotalIndex();
		stream.writeBin(totalIndex);
	}
	for (auto& fur : fc.f1) {
		glm::ivec2 pos = fur.getBotLeft();
		stream.writeBin(pos);
		ulong totalIndex = (ulong)fur.getTotalIndex();
		stream.writeBin(totalIndex);
	}
	for (auto& fur : fc.f2) {
		glm::ivec2 pos = fur.getBotLeft();
		stream.writeBin(pos);
		ulong totalIndex = (ulong)fur.getTotalIndex();
		stream.writeBin(totalIndex);
	}
	for (auto& fur : fc.f3) {
		glm::ivec2 pos = fur.getBotLeft();
		stream.writeBin(pos);
		ulong totalIndex = (ulong)fur.getTotalIndex();
		stream.writeBin(totalIndex);
	}
	for (auto& fur : fc.f4) {
		glm::ivec2 pos = fur.getBotLeft();
		stream.writeBin(pos);
		ulong totalIndex = (ulong)fur.getTotalIndex();
		stream.writeBin(totalIndex);
	}
}
