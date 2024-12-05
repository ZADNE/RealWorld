/*! 
 *  @author    Dubsky Tomas
 */
#ifndef BIOME_GLSL
#define BIOME_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/constants/tile.glsl>

const uvec2 k_stoneTiles[] = {
    k_stone,
    k_coldStone
};

struct Biome {
    uvec2 tiles;
    vec2  elevation;//x = fixed, y = random
    vec2  roughness;//x = fixed, y = slope-variable
    vec2  surfaceWidth;//x = fixed, y = slope-variable
};

//                          tile            elevation           roughness           surfaceWidth
constexpr Biome k_mountain =    {k_coldStone,   {1000., 2000.},     {400., 250.},       {800., 200.}};
constexpr Biome k_tundra =      {k_coldDirt,    {0., 1000.},        {0., 250.},         {50., 250.}};
constexpr Biome k_taiga =       {k_coldGrass,   {500., 1200.},      {0., 150.},         {50., 200.}};

constexpr Biome k_grassland =   {k_grass,       {0., 1000.},        {100., 150.},       {50., 200.}};
constexpr Biome k_forest =      {k_grass,       {0., 800.},         {100., 200.},       {50., 200.}};
constexpr Biome k_swamp =       {k_mud,         {-1500., 400.},     {0., 100.},         {500., 100.}};

constexpr Biome k_desert =      {k_sand,        {0., 500.},         {0., 0.},           {600., 100.}};
constexpr Biome k_savanna =     {k_dryGrass,    {250., 800.},       {100., 100.},       {200., 200.}};
constexpr Biome k_rainforest =  {k_mudGrass,    {250., 1000.},      {150., 160.},       {240., 280.}};

constexpr ivec2 k_biomesMatrixSize = {3, 3};
constexpr Biome k_biomes[k_biomesMatrixSize.y][k_biomesMatrixSize.x] = {
//humidity> |low                |normal             |high           temperature \/
    {       k_mountain,         k_tundra,           k_taiga         },  //-low
    {       k_grassland,        k_forest,           k_swamp         },  //-normal
    {       k_desert,           k_savanna,          k_rainforest    }   //-high
};

/**
 * @param biomeClimate x = temperature, y = humidity
 */
Biome calcBiomeStructure(vec2 biomeClimate){
    // Calculate coords
    biomeClimate = fract(biomeClimate);
    biomeClimate *= vec2(k_biomesMatrixSize - 1);
    ivec2 ll = ivec2(biomeClimate);
    vec2 frac = fract(biomeClimate);

    // Gather
    Biome b00 = k_biomes[ll.x][ll.y];
    Biome b01 = k_biomes[ll.x][ll.y + 1];
    Biome b10 = k_biomes[ll.x + 1][ll.y];
    Biome b11 = k_biomes[ll.x + 1][ll.y + 1];

    // Interpolate over X axis
    b00.elevation = mix(b00.elevation, b10.elevation, frac.x);
    b01.elevation = mix(b01.elevation, b11.elevation, frac.x);
    b00.roughness = mix(b00.roughness, b10.roughness, frac.x);
    b01.roughness = mix(b01.roughness, b11.roughness, frac.x);
    b00.surfaceWidth = mix(b00.surfaceWidth, b10.surfaceWidth, frac.x);
    b01.surfaceWidth = mix(b01.surfaceWidth, b11.surfaceWidth, frac.x);

    // Interpolate over Y axis
    b00.elevation = mix(b00.elevation, b01.elevation, frac.y);
    b00.roughness = mix(b00.roughness, b01.roughness, frac.y);
    b00.surfaceWidth = mix(b00.surfaceWidth, b01.surfaceWidth, frac.y);
    return b00;
}

#endif // !BIOME_GLSL