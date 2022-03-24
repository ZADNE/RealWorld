#pragma once

#include <RealWorld/world/WorldData.hpp>

/**
 * Static class used for both saving and loading world data (not chunks).
 * \see ChunkLoader for saving/loading chunks.
 */
class WorldDataLoader {
public:

	//Returns false if loading failed, error is printed in console
	static bool loadWorldData(WorldData& data, const std::string& worldName);

	//Returns false if saving failed, error is printed in console
	static bool saveWorldData(const WorldData& data, const std::string& worldName, bool creatingNew);

	static bool deleteWorld(const std::string& worldName);

	//All valid names are put inside vector names (previous contents of the vector are cleared)
	static void getSavedWorlds(std::vector<std::string>& names);

private:
	static void loadInfo(WorldInfo& wi, const std::string& path);
	//static void loadSurface(WorldSurface& ws, const std::string& path);
	static void loadPlayer(PlayerData& pd, const std::string& path);
	static void loadInventoryData(InventoryData& id, const std::string& path);

	static void saveInfo(const WorldInfo& wi, const std::string& path);
	//static void saveSurface(const WorldSurface& ws, const std::string& path);
	static void savePlayer(const PlayerData& pd, const std::string& path);
	static void saveInventoryData(const InventoryData& id, const std::string& path);

	static std::string m_saveFolder;
};