










//x = The properties that neighbors MUST have to transform
//y = The properties that neighbors MUST NOT have to transform
//z = Properties of the transformation
//w = The wall that it will be transformed into
const uvec4 WALL_TRANSF_RULE[] = {
//  REQUIRED properties of neighbors        FORBIDDEN properties of neighbors       RULE's properties                    TARGET tile
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 GRASS_WL},//Dirt to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 COLD_GRASS_WL},//Cold dirt to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},
    {GRASSb,                                BURNINGb,                               EDGEb | MIX_PROPSb,                 MUD_GRASS_WL},//Mud to grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_STONE_WL},//Hallow stones
    {BURNINGb,                              0,                                      MIX_PROPSb,                         DIRT_WL},//Burning of grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_WL},//Hallow grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},//Hallow dirt
    {BURNINGb,                              0,                                      MIX_PROPSb,                         COLD_DIRT_WL},//Burning of cold grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_WL},//Hallow cold grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},//Hallow dirt
    {BURNINGb,                              0,                                      MIX_PROPSb,                         MUD_WL},//Burning of mud grass
    {HALLOWb,                               0,                                      EDGEb | MIX_PROPSb,                 HALLOW_GRASS_WL},//Hallow mud grass
    {HALLOWb,                               0,                                      MIX_PROPSb,                         HALLOW_DIRT_WL},//Hallow dirt
};
