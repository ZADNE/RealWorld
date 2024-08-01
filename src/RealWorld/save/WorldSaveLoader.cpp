/*!
 *  @author    Dubsky Tomas
 */
#include <filesystem>
#include <fstream>
#include <iostream>

#include <RealEngine/utility/Error.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/save/WorldSaveLoader.hpp>

#include <nlohmann/json.hpp>

namespace rw {

template<typename T>
void writeBinary(std::ofstream& file, const T& x) {
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
}

template<typename T>
void readBinary(std::ifstream& file, T& x) {
    file.read(reinterpret_cast<char*>(&x), sizeof(x));
}

const std::string k_worldInfoFilename = "world_info.json";

bool WorldSaveLoader::createWorld(std::string worldName, int seed) {
    WorldSave save;
    // World info
    save.metadata.seed      = seed;
    save.metadata.worldName = worldName;
    save.metadata.timeDay   = 0.0f;

    // Player data
    save.player.pos = iChunkTi * glm::ivec2(0, 5) * iTilePx;
    save.inventory.resize({10, 4});

    int slot               = 0;
    save.inventory(slot++) = Item{ItemId::PCreativePickaxe, 1};
    save.inventory(slot++) = Item{ItemId::HCreativeHammer, 1};

    return saveWorld(save, worldName, true);
}

bool WorldSaveLoader::loadWorld(WorldSave& save, const std::string& worldName) {
    std::string pathToFolder = s_saveFolder + '/' + worldName + '/';
    save.metadata.path       = pathToFolder;

    try {
        loadMetadata(save.metadata, pathToFolder);
        loadPlayer(save.player, pathToFolder);
        loadInventory(save.inventory, pathToFolder);
    } catch (...) { return false; }
    return true;
}

bool WorldSaveLoader::saveWorld(
    const WorldSave& save, const std::string& worldName, bool creatingNew
) {
    if (worldName == "")
        return false;
    std::string pathToFolder = s_saveFolder + '/' + save.metadata.worldName + '/';
    bool alreadyExists = std::filesystem::exists(pathToFolder);
    if (alreadyExists && creatingNew)
        return false;

    if (!alreadyExists) {
        std::filesystem::create_directory(pathToFolder);
    }

    try {
        saveMetadata(save.metadata, pathToFolder);
        savePlayer(save.player, pathToFolder);
        saveInventory(save.inventory, pathToFolder);
    } catch (...) { return false; }
    return true;
}

bool WorldSaveLoader::deleteWorld(const std::string& worldName) {
    return std::filesystem::remove_all(s_saveFolder + '/' + worldName) > 0;
}

void WorldSaveLoader::searchSavedWorlds(std::vector<std::string>& names) {
    names.clear();
    if (!std::filesystem::is_directory(s_saveFolder)) {
        std::filesystem::create_directory(s_saveFolder);
    }

    for (const auto& entry : std::filesystem::directory_iterator(s_saveFolder)) {
        names.push_back(entry.path().filename().string());
    }
}

void WorldSaveLoader::loadMetadata(MetadataSave& metadata, const std::string& path) {
    std::ifstream i(path + k_worldInfoFilename);
    nlohmann::json j;
    i >> j;
    metadata.worldName = j["world"]["name"].get<std::string>();
    metadata.seed      = j["world"]["seed"].get<int>();
    metadata.timeDay   = j["world"]["timeDay"].get<float>();
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

    if (fileSize < sizeof(player.pos))
        throw std::exception{};

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

    if ((size_t)fileSize < sizeof(newSize))
        throw std::exception{};
    readBinary(stream, newSize);

    if ((size_t)fileSize <
        ((size_t)newSize.x * (size_t)newSize.y * sizeof(Item) + sizeof(newSize)))
        throw std::exception{};

    // All checks passed, loading inventory
    inventory.resize(newSize);

    for (int i = 0; i < inventory.slotCount(); ++i) {
        readBinary(stream, inventory(i));
    }
}

void WorldSaveLoader::saveMetadata(
    const MetadataSave& metadata, const std::string& path
) {
    nlohmann::json j = {
        {"world",
         {{"name", metadata.worldName},
          {"seed", metadata.seed},
          {"timeDay", metadata.timeDay}}}
    };

    std::ofstream o(path + k_worldInfoFilename, std::ofstream::trunc);
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

void WorldSaveLoader::saveInventory(
    const InventoryData& inventory, const std::string& path
) {
    std::ofstream stream(
        path + "player_inventory.rin", std::ios::binary | std::ios::trunc
    );
    if (stream.fail()) {
        perror(path.c_str());
        throw std::exception{};
    }

    writeBinary(stream, inventory.dims());

    for (int i = 0; i < inventory.slotCount(); ++i) {
        writeBinary(stream, inventory(i));
    }
}

} // namespace rw
