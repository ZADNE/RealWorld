/*! 
 *  @author    Dubsky Tomas
 */
#ifndef BIOME_GLSL
#define BIOME_GLSL

#include <RealWorld/constants/tile.glsl>

const uvec4 STONE_TILES[] = {
    STONE,
    COLD_STONE
};

struct Biome {
    uvec4 tiles;
    vec2 elevation;//x = fixed, y = random
    vec2 roughness;//x = fixed, y = slope-variable
    vec2 surfaceWidth;//x = fixed, y = slope-variable
};

//                          tile            elevation           roughness           surfaceWidth
const Biome MOUNTAINS =     {COLD_STONE,    {1000., 2000.},     {400., 250.},       {800., 200.}};
const Biome TUNDRA =        {COLD_DIRT,     {0., 1000.},        {0., 250.},         {50., 250.}};
const Biome TAIGA =         {COLD_GRASS,    {500., 1200.},      {0., 150.},         {50., 200.}};

const Biome DIRTLAND =      {DIRT,          {0., 1000.},        {100., 150.},       {50., 200.}};
const Biome GRASSLAND =     {GRASS,         {0., 800.},         {100., 200.},       {50., 200.}};
const Biome SWAMP =         {MUD,           {-1500., 400.},     {0., 100.},         {500., 100.}};

const Biome DESERT =        {SAND,          {0., 500.},         {0., 0.},           {600., 100.}};
const Biome SAVANNA =       {DRY_GRASS,     {250., 800.},       {100., 100.},       {200., 200.}};
const Biome RAINFOREST =    {MUD_GRASS,     {250., 1000.},      {150., 160.},       {240., 280.}};

const Biome k_biomes[3][3] = {
//humidity> |low                |normal             |high            temperature \/
    {       MOUNTAINS,          TUNDRA,             TAIGA           },  //-low
    {       DIRTLAND,           GRASSLAND,          SWAMP           },  //-normal
    {       DESERT,             SAVANNA,            RAINFOREST      }   //-high
};

/**
 * @param biomeClimate x = temperature, y = humidity
 */
Biome biomeStructure(vec2 biomeClimate){
    biomeClimate = fract(biomeClimate);
    biomeClimate *= vec2(k_biomes.length() - 1, k_biomes[0].length() - 1);
    ivec2 ll = ivec2(biomeClimate);
    vec2 frac = fract(biomeClimate);
    //Gather
    Biome b00 = k_biomes[ll.x][ll.y];
    Biome b01 = k_biomes[ll.x][ll.y + 1];
    Biome b10 = k_biomes[ll.x + 1][ll.y];
    Biome b11 = k_biomes[ll.x + 1][ll.y + 1];
    //Interpolate over X axis
    b00.elevation = mix(b00.elevation, b10.elevation, frac.x);
    b01.elevation = mix(b01.elevation, b11.elevation, frac.x);
    b00.roughness = mix(b00.roughness, b10.roughness, frac.x);
    b01.roughness = mix(b01.roughness, b11.roughness, frac.x);
    b00.surfaceWidth = mix(b00.surfaceWidth, b10.surfaceWidth, frac.x);
    b01.surfaceWidth = mix(b01.surfaceWidth, b11.surfaceWidth, frac.x);
    //Interpolate over Y axis
    b00.elevation = mix(b00.elevation, b01.elevation, frac.y);
    b00.roughness = mix(b00.roughness, b01.roughness, frac.y);
    b00.surfaceWidth = mix(b00.surfaceWidth, b01.surfaceWidth, frac.y);
    return b00;
}

#endif // BIOME_GLSL