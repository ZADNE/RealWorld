/**
 *  @author    Dubsky Tomas
 */
export module realworld:save.WorldSave;
import std;
import glm;
import :items.InventoryData;

export namespace rw {

struct MetadataSave {
    std::string path; // Includes ending slash
    int seed{};
    std::string worldName;
    float timeD{};
};

struct PlayerSave {
    glm::ivec2 pos{};
};

struct WorldSave {
    MetadataSave metadata;
    PlayerSave player;
    InventoryData inventory;
};

} // namespace rw
