/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/common.hpp>
#include <glm/vec2.hpp>

// Physics steps per second
constexpr int k_physicsStepsPerSecond = 100;

/**
 * @brief Dimensions of a tile, in pixels
 */
constexpr glm::uvec2 uTilePx = glm::uvec2(4u, 4u);
constexpr glm::ivec2 iTilePx = uTilePx;
constexpr glm::vec2  TilePx  = uTilePx;

enum class Block : uint8_t {
    Stone,
    Dirt,
    Grass,
    ColdStone,
    Sand,
    ColdDirt,
    ColdGrass,
    Mud,
    MudGrass,
    DryGrass,
    HallowStone,
    HallowDirt,
    HallowGrass,

    Water = 224,
    Lava,
    Steam,
    Fire,
    Smoke,

    Acid = 254,

    Air = 255
};

enum class Wall : uint8_t {
    Stone,
    Dirt,
    Grass,
    ColdStone,
    Sand,
    ColdDirt,
    ColdGrass,
    Mud,
    MudGrass,
    DryGrass,
    HallowStone,
    HallowDirt,
    HallowGrass,

    Air = 255
};

enum class TileAttrib : uint32_t {
    BlockType = 0,
    BlockVar  = 1,
    WallType  = 2,
    WallVar   = 3
};

enum class TileLayer : uint32_t {
    BlockLayer = static_cast<uint32_t>(TileAttrib::BlockType),
    WallLayer  = static_cast<uint32_t>(TileAttrib::WallType)
};

/**
 * @brief Converts a position in pixels to position in tiles
 */
inline glm::vec2 pxToTi(const glm::vec2& posPx) {
    return glm::floor(posPx / TilePx);
}

/**
 * @brief Converts a position in tiles to position in pixels
 */
constexpr inline glm::vec2 tiToPx(const glm::vec2& posTi) {
    return posTi * TilePx;
}
