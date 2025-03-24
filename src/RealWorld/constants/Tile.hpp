/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bit>
#include <cmath>
#include <cstdint>
#include <utility>

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

constexpr uint8_t k_nonSolidsMask = 0b0001'1111;

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
    LastUsedSolid = HallowGrass,

    Highlighter   = 223,
    FirstNonsolid = 224,
    Water         = FirstNonsolid,
    Lava,
    Steam,
    Fire,
    Smoke,
    DroppedLeaf,
    DroppedDryLeaf,
    LastUsedNonsolid = DroppedDryLeaf,

    Acid = 253,

    Air = 255
};
static_assert(
    (std::to_underlying(Block::FirstNonsolid) ^ k_nonSolidsMask) ==
        std::underlying_type_t<Block>(~0),
    "Nonsolids outside bitmask"
);

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
    LastUsedSolid = HallowGrass,

    Highlighter   = 223,
    FirstNonsolid = 224, // 0b1110'0000
    OakWood       = FirstNonsolid,
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
    LastUsedNonsolid = HallowWood,

    Air = 255
};
static_assert(
    (std::to_underlying(Wall::FirstNonsolid) ^ k_nonSolidsMask) ==
        std::underlying_type_t<Wall>(~0),
    "Nonsolids outside bitmask"
);

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

// Ranges for variants
constexpr glm::uint k_solidTileInnerVariantCount = 12;
constexpr glm::uint k_solidTileOuterVariantCount = 4;
constexpr glm::uint k_solidTileOuterVariantMask = k_solidTileOuterVariantCount - 1;

constexpr glm::uint k_nonsolidTileVariantCount = 16;
constexpr glm::uint k_nonsolidTileVariantMask  = k_nonsolidTileVariantCount - 1;

} // namespace rw
