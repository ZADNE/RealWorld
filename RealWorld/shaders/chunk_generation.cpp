#include <RealWorld/shaders/chunk_generation.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view chunkGen_vert =
#include "chunk_generation/ChunkUniforms.glsl"
#include "chunk_generation/chunkGen.vert"
;

std::string_view basicTerrain_frag =
#include "external/hash.glsl"
#include "external/snoise.glsl"
#include "common/tileConstants.glsl"
#include "chunk_generation/columnValues.glsl"
#include "chunk_generation/ChunkUniforms.glsl"
#include "chunk_generation/basicTerrain.frag"
;

std::string_view cellularAutomaton_frag =
#include "common/tileConstants.glsl"
#include "chunk_generation/cellularAutomaton.frag"
;

std::string_view selectVariation_frag =
#include "common/tileConstants.glsl"
#include "chunk_generation/selectVariation.frag"
;
