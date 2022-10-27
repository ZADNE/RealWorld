#include <RealWorld/constants/tile.glsl>

const int ANALYSIS_GROUP_SIZE = 8;

const int CALC_GROUP_SIZE = 8;

const int LIGHT_SCALE = 4;
const int LIGHT_SCALE_BITS = 2;
const float LIGHT_SCALE_INV = 0.25;
const int LIGHT_MAX_RANGEUn = LIGHT_MAX_RANGETi >> LIGHT_SCALE_BITS;
