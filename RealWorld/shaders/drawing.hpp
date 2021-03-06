/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string_view>

const int LOC_POSITIONPx = 3;
const int LOC_POSITIONTi = 4;
const int LOC_LIGHT_COUNT = 5;

extern std::string_view analysis_comp;

extern std::string_view addDynamicLights_comp;

extern std::string_view calcShadows_comp;

extern std::string_view drawTiles_vert;

extern std::string_view colorDraw_frag;

extern std::string_view drawShadows_vert;

extern std::string_view minimap_vert;

extern std::string_view minimap_frag;