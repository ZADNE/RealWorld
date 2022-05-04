#include <RealWorld/save/WorldSaveLoader.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>

#include <SDL2/SDL_timer.h>
#include <nlohmann/json.hpp>
#include <magic_enum/magic_enum.hpp>

#include <lodepng/lodepng.hpp>
#include <RealEngine/main/Error.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/performance_tests/switches.hpp>

template<class T>
void writeBinary(std::ofstream& file, const T& x) {
	file.write(reinterpret_cast<const char*>(&x), sizeof(x));
}

template<class T>
void readBinary(std::ifstream& file, T& x) {
	file.read(reinterpret_cast<char*>(&x), sizeof(x));
}

std::string WorldSaveLoader::m_saveFolder = "saves";

const std::string WORLD_INFO_FILENAME = "world_info.json";


bool WorldSaveLoader::createWorld(std::string worldName, int seed) {
	WorldSave save;
	//World info
	save.metadata.seed = seed;
	save.metadata.worldName = worldName;

	//Player data
#ifdef MEASURE_GENERATION_DELAY
	save.player.pos = glm::ivec2(0, 0);
#else
	save.player.pos = iCHUNK_SIZE * glm::ivec2(0, 5) * iTILEPx;
#endif // MEASURE_GENERATION_DELAY
	save.inventory.resize({10, 4});

	int slot = 0;
	save.inventory(slot++) = Item{ITEM::CREATIVE_PICKAXE, 1};
	save.inventory(slot++) = Item{ITEM::CREATIVE_HAMMER, 1};

	for (size_t i = magic_enum::enum_integer(ITEM::F_WATER); i <= magic_enum::enum_integer(ITEM::W_DRY_GRASS); ++i) {
		save.inventory(slot++) = Item{static_cast<ITEM>(i), 1};
	}

	return saveWorld(save, worldName, true);
}

bool WorldSaveLoader::loadWorld(WorldSave& save, const std::string& worldName) {
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + worldName + "/";
	save.metadata.path = pathToFolder;

	try {
		loadMetadata(save.metadata, pathToFolder);
		loadPlayer(save.player, pathToFolder);
		loadInventory(save.inventory, pathToFolder);
	}
	catch (...) {
		return false;
	}

	std::cout << "Successfully loaded world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldSaveLoader::saveWorld(const WorldSave& save, const std::string& worldName, bool creatingNew) {
	if (worldName == "") return false;
	unsigned int ticks = SDL_GetTicks();
	std::string pathToFolder = m_saveFolder + "/" + save.metadata.worldName + "/";
	bool alreadyExists = std::filesystem::exists(pathToFolder);
	if (alreadyExists && creatingNew) return false;

	if (!alreadyExists) {
		std::filesystem::create_directory(pathToFolder);
	}

	try {
		saveMetadata(save.metadata, pathToFolder);
		savePlayer(save.player, pathToFolder);
		saveInventory(save.inventory, pathToFolder);
	}
	catch (...) {
		return false;
	}

	std::cout << "Successfully saved world " << worldName << " in " << SDL_GetTicks() - ticks << " ms.\n";
	return true;
}

bool WorldSaveLoader::deleteWorld(const std::string& worldName) {
	if (std::filesystem::remove_all(m_saveFolder + "/" + worldName) > 0) {
		return true;
	} else {
		return false;
	}
}

void WorldSaveLoader::getSavedWorlds(std::vector<std::string>& names) {
	names.clear();
	if (!std::filesystem::is_directory(m_saveFolder)) {
		std::filesystem::create_directory(m_saveFolder);
	}

	for (const auto& entry : std::filesystem::directory_iterator(m_saveFolder)) {
		names.push_back(entry.path().filename().string());
	}
}

void WorldSaveLoader::loadMetadata(MetadataSave& metadata, const std::string& path) {
	std::ifstream i(path + WORLD_INFO_FILENAME);
	nlohmann::json j;
	i >> j;
	metadata.worldName = j["world"]["name"].get<std::string>();
	metadata.seed = j["world"]["seed"].get<int>();
}

void WorldSaveLoader::loadPlayer(PlayerSave& player, const std::string& path) {
	std::ifstream stream(path + "player_state.rst", std::ios::binary);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}

	stream.seekg(0, std::ios::end);
	auto fileSize = stream.tellg();
	stream.seekg(0, std::ios::beg);

	if (fileSize < sizeof(player.pos)) throw std::exception{};

	readBinary(stream, player.pos);
}

void WorldSaveLoader::loadInventory(InventoryData& inventory, const std::string& path) {
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
	inventory.resize(newSize);

	for (int i = 0; i < inventory.slotCount(); ++i) {
		readBinary(stream, inventory(i));
	}
}

void WorldSaveLoader::saveMetadata(const MetadataSave& metadata, const std::string& path) {
	nlohmann::json j = {
		{"world", {
			{"name", metadata.worldName},
			{"seed", metadata.seed}
		}}
	};

	std::ofstream o(path + WORLD_INFO_FILENAME, std::ofstream::trunc);
	o << j.dump(2);
	o.close();
}

void WorldSaveLoader::savePlayer(const PlayerSave& playerSave, const std::string& path) {
	std::ofstream stream(path + "player_state.rst", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror((path + "player_state.rst").c_str());
		throw std::exception{};
	}
	writeBinary(stream, playerSave.pos);
}

void WorldSaveLoader::saveInventory(const InventoryData& inventory, const std::string& path) {
	std::ofstream stream(path + "player_inventory.rin", std::ios::binary | std::ios::trunc);
	if (stream.fail()) {
		perror(path.c_str());
		throw std::exception{};
	}

	writeBinary(stream, inventory.getSize());

	for (int i = 0; i < inventory.slotCount(); ++i) {
		writeBinary(stream, inventory(i));
	}
}
