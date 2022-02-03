#include <RealWorld/shaders/chunk_generation.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view chunkGen_vert =
#include "chunk_generation/chunkGen.vert"
;

std::string_view basicTerrain_frag =
#include "chunk_generation/basicTerrain.frag"
;

std::string_view selectVariation_frag =
#include "chunk_generation/selectVariation.frag"
;

std::string_view cellularAutomaton_frag =
#include "chunk_generation/cellularAutomaton.frag"
;
