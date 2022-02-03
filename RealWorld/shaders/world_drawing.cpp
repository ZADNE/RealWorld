#include <RealWorld/shaders/world_drawing.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view tilesDraw_vert =
#include "world_drawing/tilesDraw.vert"
;

std::string_view tilesDraw_frag =
#include "world_drawing/tilesDraw.frag"
;

std::string_view finalLighting_vert =
#include "world_drawing/finalLighting.vert"
;

std::string_view PT_vert =
#include "world_drawing/PT.vert"
;

std::string_view combineLighting_frag =
#include "world_drawing/combineLighting.frag"
;

std::string_view worldToLight_frag =
#include "world_drawing/worldToLight.frag"
;

std::string_view addDynamicLight_vert =
#include "world_drawing/addDynamicLight.vert"
;

std::string_view addDynamicLight_frag =
#include "world_drawing/addDynamicLight.frag"
;

std::string_view worldPT_vert =
#include "world_drawing/worldPT.vert"
;

std::string_view minimap_vert =
#include "world_drawing/minimap.vert"
;

std::string_view minimap_frag =
#include "world_drawing/minimap.frag"
;