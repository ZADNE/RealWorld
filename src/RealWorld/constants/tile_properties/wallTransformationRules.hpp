/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationRules.hpp>

namespace rw {

using WallTransformationRules = std::array<glm::uvec4, 25>;

// clang-format off: Custom alignment of columns

// x = The properties that neighbors MUST have to transform
// y = The properties that neighbors MUST NOT have to transform
// z = Properties of the transformation
// w = The wall that it will be transformed into
constexpr WallTransformationRules k_wallTransformationRules = {
//              REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,           Wall::Grass},           // Dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,           Wall::ColdGrass},       // Cold dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               1023 | EDGEb | MIX_PROPSb,           Wall::MudGrass},        // Mud to grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowStone},     // Hallow stones
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                   Wall::Dirt},            // Burning of grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,           Wall::HallowGrass},     // Hallow grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},      // Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                   Wall::ColdDirt},        // Burning of cold grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,           Wall::HallowGrass},     // Hallow cold grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},      // Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      1023 | MIX_PROPSb,                   Wall::Mud},             // Burning of mud grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | EDGEb | MIX_PROPSb,           Wall::HallowGrass},     // Hallow mud grass
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowDirt},      // Hallow dirt
    glm::uvec4{ BURNINGb,                               EXTINGUISHINGb,                         1023 | MIX_PROPSb | TO_OTHERb,       Block::Fire},           // Spark from burning of leaf
    glm::uvec4{ 0,                                      0,                                      63   | EDGEb | TO_OTHERb,            Block::DroppedLeaf},    // Leaf drop
    glm::uvec4{ BURNINGb,                               EXTINGUISHINGb,                         1023 | MIX_PROPSb | TO_OTHERb,       Block::Fire},           // Spark from burning of dry leaf
    glm::uvec4{ 0,                                      0,                                      63   | EDGEb | TO_OTHERb,            Block::DroppedDryLeaf}, // Dry leaf drop
    glm::uvec4{ BURNINGb,                               EXTINGUISHINGb,                         1023 | MIX_PROPSb,                   Wall::BurningWood},     // Burning of wood
    glm::uvec4{ HALLOWb,                                0,                                      1023 | MIX_PROPSb,                   Wall::HallowWood},      // Hallow of wood
    glm::uvec4{ EXTINGUISHINGb,                         0,                                      1023 | MIX_PROPSb,                   Wall::BurntWood},       // Extinguishing of wood
    glm::uvec4{ 0,                                      0,                                      383  | MIX_PROPSb,                   Wall::BurntWood},       // Extinguishing of wood
    glm::uvec4{ 0,                                      EXTINGUISHINGb,                         1023 | MIX_PROPSb | TO_OTHERb,       Block::Fire},           // Spark from burning of wood
};

// clang-format on

} // namespace rw
