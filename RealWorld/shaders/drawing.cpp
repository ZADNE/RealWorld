/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/shaders/drawing.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view analysis_comp =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "reserved_units/images.glsl"
#include "drawing/analysis.comp"
;

std::string_view calcShadows_comp =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "reserved_units/images.glsl"
#include "drawing/calc_shadows.comp"
;

std::string_view drawTiles_vert =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "drawing/WorldDrawUBO.glsl"
#include "drawing/draw_tiles.vert"
;

std::string_view colorDraw_frag =
#include "drawing/color_draw.frag"
;

std::string_view drawShadows_vert =
#include "common/tile_constants.glsl"
#include "reserved_units/textures.glsl"
#include "drawing/WorldDrawUBO.glsl"
#include "drawing/draw_shadows.vert"
;

std::string_view minimap_vert =
#include "drawing/minimap.vert"
;

std::string_view minimap_frag =
#include "reserved_units/textures.glsl"
#include "drawing/minimap.frag"
;