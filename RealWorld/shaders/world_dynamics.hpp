#pragma once
#include <string_view>

const int LOC_SET = 3;

extern std::string_view setWithUpdate_vert;

extern std::string_view setWithUpdate_frag;

const int LOC_GLOBAL_OFFSET = 0;

const int LOC_MODIFY_TARGET = 1;
const int LOC_MODIFY_SHAPE = 2;
const int LOC_MODIFY_DIAMETER = 3;
const int LOC_MODIFY_TILE = 4;

extern std::string_view modify_comp;

const int LOC_TIME_HASH = 1;
const int LOC_UPDATE_ORDER = 2;

extern std::string_view dynamics_comp;

extern std::string_view transform_comp;

extern std::string_view playerDynamics_comp;