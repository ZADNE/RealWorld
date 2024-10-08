﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bit>
#include <cmath>
#include <cstdint>

#include <glm/common.hpp>
#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief The number of physics steps per second
 */
constexpr int k_physicsStepsPerSecond = 100;
constexpr int k_physicsStepsPerMinute = k_physicsStepsPerSecond * 60;

/**
 * @brief Dimensions of a tile, in pixels
 */
constexpr glm::uvec2 uTilePx = glm::uvec2(4u, 4u);
constexpr glm::ivec2 iTilePx = uTilePx;
constexpr glm::vec2 TilePx   = uTilePx;

static_assert(std::has_single_bit(uTilePx.x) && std::has_single_bit(uTilePx.y));

constexpr glm::ivec2 k_tileLowZeroBits =
    glm::ivec2(std::countr_zero(uTilePx.x), std::countr_zero(uTilePx.y));

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

    Highlighter = 223,
    Water       = 224,
    Lava,
    Steam,
    Fire,
    Smoke,
    DroppedLeaf,
    DroppedDryLeaf,

    Acid = 253,

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

    Highlighter = 223,
    OakWood     = 224, // 0b1110'0000
    AcaciaWood,
    ConiferousWood,
    PalmWood,

    TallGrass = 240,
    ColdTallGrass,
    MudTallGrass,
    DryTallGrass,
    Leaf,
    DryLeaf,
    Needle,
    PalmLeaf,
    Withy,
    Cactus,
    BurningWood,
    BurntWood,
    HallowWood,

    Air = 255
};

enum class TileLayer : uint32_t {
    Block = 0,
    Wall  = 1
};
constexpr uint32_t k_tileLayerCount = 2;

enum class TileAttrib : uint32_t {
    BlockType = 0,
    BlockVar  = 1,
    WallType  = 0,
    WallVar   = 1
};

/**
 * @brief Converts a position in pixels to position in tiles
 */
constexpr glm::vec2 pxToTi(glm::vec2 posPx) {
    glm::vec2 posTiFrac = posPx / TilePx;
    return {std::floor(posTiFrac.x), std::floor(posTiFrac.y)};
}
constexpr glm::ivec2 pxToTi(glm::ivec2 posPx) {
    return posPx >> k_tileLowZeroBits;
}

/**
 * @brief Converts a position in tiles to position in pixels
 */
constexpr glm::vec2 tiToPx(glm::vec2 posTi) {
    return posTi * TilePx;
}
constexpr glm::ivec2 tiToPx(glm::ivec2 posTi) {
    return posTi << k_tileLowZeroBits;
}

} // namespace rw
