/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationRules.hpp>

namespace rw {




//x = The properties that neighbors MUST have to transform
//y = The properties that neighbors MUST NOT have to transform
//z = Properties of the transformation
//w = The wall that it will be transformed into
constexpr std::array<glm::uvec4, 16> k_wallTransformationRules = { 
//              REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Wall::Grass},//Dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Wall::ColdGrass},//Cold dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 Wall::MudGrass},//Mud to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowStone},//Hallow stones
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Wall::Dirt},//Burning of grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Wall::HallowGrass},//Hallow grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Wall::ColdDirt},//Burning of cold grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Wall::HallowGrass},//Hallow cold grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         Wall::Mud},//Burning of mud grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 Wall::HallowGrass},//Hallow mud grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         Wall::HallowDirt},//Hallow dirt
};

} // namespace rw