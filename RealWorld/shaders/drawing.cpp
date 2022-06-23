/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/shaders/drawing.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view tilesDraw_vert =
#include "common/tile_constants.glsl"
#include "common/texture_units.glsl"
#include "drawing/WorldDrawUBO.glsl"
#include "drawing/tiles_draw.vert"
;

std::string_view colorDraw_frag =
#include "drawing/color_draw.frag"
;

std::string_view coverWithShadows_vert =
#include "common/tile_constants.glsl"
#include "common/texture_units.glsl"
#include "drawing/cover_with_shadows.vert"
;

std::string_view minimap_vert =
#include "drawing/minimap.vert"
;

std::string_view minimap_frag =
#include "common/texture_units.glsl"
#include "drawing/minimap.frag"
;