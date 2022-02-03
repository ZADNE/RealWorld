#pragma once
#include <string_view>

const int LOC_POSITION = 3;

extern std::string_view tilesDraw_vert;

extern std::string_view tilesDraw_frag;

extern std::string_view finalLighting_vert;

extern std::string_view PT_vert;

const int LOC_DIAPHRAGM = 100;

const int LOC_LIGHTING = 101;

extern std::string_view combineLighting_frag;

const int LOC_WORLD_TO_LIGHT_DAYLIGHT = 4;

const int LOC_WORLD_TO_LIGHT_DIAPHRAGMS = 5;

extern std::string_view worldToLight_frag;

extern std::string_view addDynamicLight_vert;

extern std::string_view addDynamicLight_frag;

extern std::string_view worldPT_vert;

extern std::string_view minimap_vert;

extern std::string_view minimap_frag;