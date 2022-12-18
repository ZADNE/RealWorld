struct DynamicLight{
    ivec2 posPx;
    uint col;
    uint padding;
};

#include <RealWorld/reserved_units/buffers.glsl>
layout(std430, binding = STRG_BUF_EXTERNALLIGHTS) readonly restrict buffer DynamicLightsSSIB {
    DynamicLight lights[];
};
