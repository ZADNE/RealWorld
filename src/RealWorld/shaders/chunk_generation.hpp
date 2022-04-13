#pragma once
#include <string_view>

const int LOC_THRESHOLDS = 33;
const int LOC_CYCLE_N = 34;


extern std::string_view chunkGen_vert;
extern std::string_view basicTerrain_frag;
extern std::string_view cellularAutomaton_frag;
extern std::string_view selectVariation_frag;


extern std::string_view basicTerrain_comp;
extern std::string_view cellularAutomaton_comp;
extern std::string_view selectVariation_comp;