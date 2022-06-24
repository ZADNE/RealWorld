/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/shaders/generation.hpp>

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
#include "generation/structure_constants.glsl"
#include "generation/column_values.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/structure.glsl"
#include "generation/structure.frag"
;

std::string_view consolidation_frag =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "generation/consolidation.frag"
;

std::string_view variationSelection_frag =
#include "external/float_hash.glsl"
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "generation/variant_selection.frag"
;

std::string_view structure_comp =
#include "external/float_hash.glsl"
#include "external/snoise.glsl"
#include "reserved_units/images.glsl"
#include "common/tile_constants.glsl"
#include "generation/structure_constants.glsl"
#include "generation/gen_settings.glsl"
#include "generation/column_values.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/structure.glsl"
#include "generation/structure.comp"
;

std::string_view consolidation_comp =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "reserved_units/images.glsl"
#include "generation/gen_settings.glsl"
#include "generation/consolidation.comp"
;

std::string_view variantSelection_comp =
#include "external/float_hash.glsl"
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "reserved_units/images.glsl"
#include "generation/gen_settings.glsl"
#include "generation/ChunkUBO.glsl"
#include "generation/variant_selection.comp"
;