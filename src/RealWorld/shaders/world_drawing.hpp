#pragma once
#include <string_view>

const int LOC_POSITION = 3;

extern std::string_view tilesDraw_vert;

extern std::string_view colorDraw_frag;

extern std::string_view coverWithDarkness_vert;

extern std::string_view PT_vert;

extern std::string_view computeLighting_frag;

const int LOC_WORLD_TO_LIGHT_DAYLIGHT = 4;

const int LOC_WORLD_TO_LIGHT_DIAPHRAGMS = 5;

extern std::string_view worldToLight_frag;

extern std::string_view addDynamicLight_vert;

extern std::string_view addDynamicLight_frag;

extern std::string_view minimap_vert;

extern std::string_view minimap_frag;