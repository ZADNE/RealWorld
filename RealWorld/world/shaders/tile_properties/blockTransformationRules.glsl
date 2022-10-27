




const uint EDGEb =                  1 << 0;
const uint MIX_PROPSb =             1 << 1;//Mix properties with the other layer 




//x = The properties that neighbors MUST have to transform
//y = The properties that neighbors MUST NOT have to transform
//z = Properties of the transformation
//w = The block that it will be transformed into
const uvec4 BLOCK_TRANSF_RULE[] = {
//  REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                   TARGET tile
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 GRASS_BL},//Dirt to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 COLD_GRASS_BL},//Cold dirt to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 MUD_GRASS_BL},//Mud to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_STONE_BL},//Hallow stones
    {BURNINGb,                              0,                                      MIX_PROPSb,                         DIRT_BL},//Burning of grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_BL},//Hallow grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},//Hallow dirt
    {BURNINGb,                              0,                                      MIX_PROPSb,                         COLD_DIRT_BL},//Burning of cold grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_BL},//Hallow cold grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},//Hallow dirt
    {BURNINGb,                              0,                                      MIX_PROPSb,                         MUD_BL},//Burning of mud grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_BL},//Hallow mud grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_BL},//Hallow dirt
};
