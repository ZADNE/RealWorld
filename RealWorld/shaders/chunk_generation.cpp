#include <RealWorld/shaders/chunk_generation.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view basicTerrain_comp =
#include "external/float_hash.glsl"
#include "external/snoise.glsl"
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "chunk_generation/columnValues.glsl"
#include "chunk_generation/ChunkUniforms.glsl"
#include "chunk_generation/basicTerrain.comp"
;

std::string_view cellularAutomaton_comp =
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "chunk_generation/cellularAutomaton.comp"
;

std::string_view selectVariation_comp =
#include "external/float_hash.glsl"
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "chunk_generation/ChunkUniforms.glsl"
#include "chunk_generation/selectVariation.comp"
;