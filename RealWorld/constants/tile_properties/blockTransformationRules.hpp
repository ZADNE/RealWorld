/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>
#include <RealWorld/constants/tile.hpp>

constexpr glm::uint EDGEb =                  1 << 0;
constexpr glm::uint MIX_PROPSb =             1 << 1;//Mix properties with the other layer 


//x = The properties that neighbors MUST have to transform
//y = The properties that neighbors MUST NOT have to transform
//z = Properties of the transformation
//w = The block that it will be transformed into
constexpr std::array<glm::uvec4, 16> BLOCK_TRANSFORMATION_RULES = {
//              REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 BLOCK::GRASS},//Dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 BLOCK::COLD_GRASS},//Cold dirt to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},
    glm::uvec4{ GRASSb,                                 BURNINGb,                               EDGEb | MIX_PROPSb,                 BLOCK::MUD_GRASS},//Mud to grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_STONE},//Hallow stones
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         BLOCK::DIRT},//Burning of grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 BLOCK::HALLOW_GRASS},//Hallow grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         BLOCK::COLD_DIRT},//Burning of cold grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 BLOCK::HALLOW_GRASS},//Hallow cold grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},//Hallow dirt
    glm::uvec4{ BURNINGb,                               0,                                      MIX_PROPSb,                         BLOCK::MUD},//Burning of mud grass
    glm::uvec4{ HALLOWb,                                0,                                      EDGEb | MIX_PROPSb,                 BLOCK::HALLOW_GRASS},//Hallow mud grass
    glm::uvec4{ HALLOWb,                                0,                                      MIX_PROPSb,                         BLOCK::HALLOW_DIRT},//Hallow dirt
};
