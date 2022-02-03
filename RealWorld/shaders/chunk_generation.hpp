#pragma once
#include <string_view>

const int LOC_SET = 5;

const int LOC_WORLD_TEXTURE = 2;

//Chunk generator \|/
const int LOC_TILES_SELECTOR = 1;
const int LOC_TILES0_TEXTURE = 2;
const int LOC_TILES1_TEXTURE = 3;
const int LOC_MATERIAL_TEXTURE = 4;

extern std::string_view chunkGen_vert;

extern std::string_view basicTerrain_frag;

extern std::string_view selectVariation_frag;

const int LOC_CELL_AUTO_LOW = 33;
const int LOC_CELL_AUTO_HIGH = 34;

extern std::string_view cellularAutomaton_frag;