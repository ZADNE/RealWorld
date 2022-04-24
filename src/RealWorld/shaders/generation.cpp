﻿#include <RealWorld/shaders/generation.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view chunkGen_vert =
#include "common/tile_constants.glsl"
#include "generation/gen_settings.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/chunk_gen.vert"
;

std::string_view structure_frag =
#include "external/float_hash.glsl"
#include "external/snoise.glsl"
#include "common/tile_constants.glsl"
#include "generation/column_values.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/structure.glsl"
#include "generation/structure.frag"
;

std::string_view consolidation_frag =
#include "common/tile_constants.glsl"
#include "common/texture_units.glsl"
#include "generation/consolidation.frag"
;

std::string_view variationSelection_frag =
#include "external/float_hash.glsl"
#include "common/tile_constants.glsl"
#include "common/texture_units.glsl"
#include "generation/variant_selection.frag"
;

std::string_view structure_comp =
#include "external/float_hash.glsl"
#include "external/snoise.glsl"
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "generation/gen_settings.glsl"
#include "generation/column_values.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/structure.glsl"
#include "generation/structure.comp"
;

std::string_view consolidation_comp =
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "common/texture_units.glsl"
#include "generation/gen_settings.glsl"
#include "generation/consolidation.comp"
;

std::string_view variantSelection_comp =
#include "external/float_hash.glsl"
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "common/texture_units.glsl"
#include "generation/gen_settings.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/variant_selection.comp"
;