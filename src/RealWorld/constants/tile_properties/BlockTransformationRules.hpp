﻿/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/Tile.hpp>
#include <RealWorld/constants/tile_properties/BlockTransformationProperties.hpp>

namespace rw {

using BlockTransformationRules = std::array<glm::uvec4, 16>;

// clang-format off: Custom alignment of columns

constexpr glm::uint PROBb      = 1023; // Bits used to denote the probability of the transformation to happen
constexpr glm::uint EDGEb      = 1024; // The time must be on edge
constexpr glm::uint MIX_PROPSb = 2048; // Mix properties with the other layer
constexpr glm::uint TO_OTHERb  = 4096; // Output of the rule should be placed in the other layer if there is air


// x = The properties that neighbors MUST have to transform
// y = The properties that neighbors MUST NOT have to transform
// z = Properties of the transformation
// w = The block that it will be transformed into
constexpr BlockTransformationRules k_blockTransformationRules = {
//              REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,          Block::Grass},      // Dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,          Block::ColdGrass},  // Cold dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,          Block::MudGrass},   // Mud to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt},
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowStone},// Hallow stones
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                  Block::Dirt},       // Burning of grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,          Block::HallowGrass},// Hallow grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt}, // Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                  Block::ColdDirt},   // Burning of cold grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,          Block::HallowGrass},// Hallow cold grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt}, // Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                  Block::Mud},        // Burning of mud grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,          Block::HallowGrass},// Hallow mud grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                  Block::HallowDirt}, // Hallow dirt
};

// clang-format on

} // namespace rw
