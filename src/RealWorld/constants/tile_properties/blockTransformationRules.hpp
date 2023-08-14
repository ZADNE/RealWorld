/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>
#include <RealWorld/constants/tile.hpp>

namespace rw {

constexpr glm::uint EDGEb =                  1 << 0;
constexpr glm::uint MIX_PROPSb =             1 << 1;//Mix properties with the other layer 
//x = The properties that neighbors MUST have to transform
//y = The properties that neighbors MUST NOT have to transform
//z = Properties of the transformation
//w = The block that it will be transformed into
constexpr std::array<glm::uvec4, 16> k_blockTransformationRules = {
//              REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Block::Grass},//Dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Block::ColdGrass},//Cold dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Block::MudGrass},//Mud to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowStone},//Hallow stones
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Block::Dirt},//Burning of grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Block::HallowGrass},//Hallow grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Block::ColdDirt},//Burning of cold grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Block::HallowGrass},//Hallow cold grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Block::Mud},//Burning of mud grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Block::HallowGrass},//Hallow mud grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Block::HallowDirt},//Hallow dirt
};

} // namespace rw