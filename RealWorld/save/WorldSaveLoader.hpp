/*!
 *  @author    Dubsky Tomas
 */
#pragma once

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

/**
 * @brief Saves and loads world data (except for chunks)
 *
 * @see ChunkLoader saves and loads chunks.
 */
class WorldSaveLoader {
public:
    static bool createWorld(std::string worldName, int seed);

    // Returns false if loading failed, error is printed in console
    static bool loadWorld(WorldSave& data, const std::string& worldName);

    // Returns false if saving failed, error is printed in console
    static bool saveWorld(
        const WorldSave& data, const std::string& worldName, bool creatingNew
    );

    // Returns false if the world could not be deleted
    static bool deleteWorld(const std::string& worldName);

    // All valid names are put inside vector names (previous contents of the
    // vector are cleared)
    static void searchSavedWorlds(std::vector<std::string>& names);

private:
    static void loadMetadata(MetadataSave& metadata, const std::string& path);
    static void loadPlayer(PlayerSave& player, const std::string& path);
    static void loadInventory(InventoryData& inventory, const std::string& path);

    static void saveMetadata(const MetadataSave& metadata, const std::string& path);
    static void savePlayer(const PlayerSave& pd, const std::string& path);
    static void saveInventory(const InventoryData& inventory, const std::string& path);

    static inline std::string s_saveFolder = "saves";
};

} // namespace rw