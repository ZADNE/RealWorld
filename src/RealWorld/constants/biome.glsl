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

const Biome BIOMES[3][3] = {
//humidity> |low                |normal             |high            temperature \/
    {       MOUNTAINS,          TUNDRA,             TAIGA           },  //-low
    {       DIRTLAND,           GRASSLAND,          SWAMP           },  //-normal
    {       DESERT,             SAVANNA,            RAINFOREST      }   //-high
};

#endif // BIOME_GLSL