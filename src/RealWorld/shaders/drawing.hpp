#pragma once
#include <string_view>

const int LOC_POSITION = 3;

extern std::string_view tilesDraw_vert;

extern std::string_view colorDraw_frag;

extern std::string_view coverWithShadows_vert;

extern std::string_view passthrough_vert;

extern std::string_view computeLighting_frag;

const int LOC_TILES_TO_LIGHT_DAYLIGHT = 4;

const int LOC_TILES_TO_LIGHT_DIAPHRAGMS = 5;

extern std::string_view tilesToLight_frag;

extern std::string_view addLight_vert;

extern std::string_view addLight_frag;

extern std::string_view minimap_vert;

extern std::string_view minimap_frag;