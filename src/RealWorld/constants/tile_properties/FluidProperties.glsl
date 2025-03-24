/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_FLUID_PROPERTIES_GLSL
#define RW_FLUID_PROPERTIES_GLSL
/*
    1: water, 2: lava,  3: steam,
    4: fire,  5: smoke, 6: droppedLeaf
    7: droppedDryLeaf
*/

// 0-1024 = probability to move horizontally
const uint k_fluidity[] = {
    1024,   16,     200,
    24,     64,     256,
    256
};

// -1 = moves down (liquids), 1 = moves up (gases)
const int k_primMoveDir[] = {
    -1,     -1,     1,
    1,      1,      -1,
    -1
};

// 0-1024 probability to move vertically
const uint k_primMoveProb[] = {
    920,    960,    200,
    320,    64,     128,
    128
};

// 0-1024 = probability to maintain horizontal direction against the wind
// Fluids with higher density move below fluid lower density
const uint k_density[] = {
    900,    400,    600,
    600,    600,    1010,
    1010
};

// x = 0-1024 probability to convert the fluid
// y = The block that this fluid is converted to
// z = The block that triggers conversion of this fluid when they touch
const uvec3 k_triggerConv[] = {
    {1024, k_steamBl, k_lavaBl},    {512, k_stoneBl, k_waterBl},    {0, k_airBl, k_neverBl},
    {1024, k_airBl, k_waterBl},     {1024, k_airBl, k_waterBl},     {0, k_airBl, k_neverBl},
    {0, k_airBl, k_neverBl}
};

// x = 0-65535 probability to randomly convert the fluid
// y = The block that this fluid is converted to
// z = 0-65535 probability to randomly convert the fluid if the previous test failed
// w = The block that this fluid is converted to if the previous test failed
const uvec4 k_randomConv[] = {
    {1, k_steamBl, 0, k_airBl},     {1, k_stoneBl, 1, k_smokeBl},   {64, k_waterBl, 64, k_airBl},
    {800, k_smokeBl, 0, k_airBl},   {384, k_airBl, 0, k_airBl},     {256, k_airBl, 0, k_airBl},
    {256, k_airBl, 0, k_airBl}
};

#endif // !RW_FLUID_PROPERTIES_GLSL
