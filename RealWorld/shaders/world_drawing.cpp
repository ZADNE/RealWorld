#include <RealWorld/shaders/world_drawing.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view tilesDraw_vert =
#include "common/tileConstants.glsl"
#include "common/textureUnits.glsl"
#include "world_drawing/WorldDrawUBO.glsl"
#include "world_drawing/tilesDraw.vert"
;

std::string_view colorDraw_frag =
#include "world_drawing/colorDraw.frag"
;

std::string_view coverWithDarkness_vert =
#include "common/tileConstants.glsl"
#include "common/textureUnits.glsl"
#include "world_drawing/coverWithDarkness.vert"
;

std::string_view PT_vert =
#include "world_drawing/WorldDrawUBO.glsl"
#include "world_drawing/PT.vert"
;

std::string_view computeLighting_frag =
#include "common/tileConstants.glsl"
#include "common/textureUnits.glsl"
#include "world_drawing/computeLighting.frag"
;

std::string_view worldToLight_frag =
#include "common/tileConstants.glsl"
#include "common/textureUnits.glsl"
#include "world_drawing/worldToLight.frag"
;

std::string_view addDynamicLight_vert =
#include "common/tileConstants.glsl"
#include "world_drawing/WorldDrawUBO.glsl"
#include "world_drawing/addDynamicLight.vert"
;

std::string_view addDynamicLight_frag =
#include "world_drawing/addDynamicLight.frag"
;

std::string_view minimap_vert =
#include "world_drawing/minimap.vert"
;

std::string_view minimap_frag =
#include "common/textureUnits.glsl"
#include "world_drawing/minimap.frag"
;