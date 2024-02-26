/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>

#include <glm/vec2.hpp>

#include <RealWorld/items/InventoryData.hpp>

namespace rw {

struct MetadataSave {
    std::string path; // Includes ending slash
    int seed;
    std::string worldName;
};

struct PlayerSave {
    glm::ivec2 pos;
};

struct WorldSave {
    MetadataSave metadata;
    PlayerSave player;
    InventoryData inventory;
};

} // namespace rw
