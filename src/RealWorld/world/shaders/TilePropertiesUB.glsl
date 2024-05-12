/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_PROPERTIES_UB_GLSL
#define TILE_PROPERTIES_UB_GLSL

// Tile properties
const uint STONEb     = 1 << 0;
const uint DIRTb      = 1 << 1;
const uint GRASSb     = 1 << 2;
const uint BURNINGb   = 1 << 3;
const uint HALLOWb    = 1 << 4;
const uint WOODb      = 1 << 5;

// Wall properties
const uint PROBb      = 1023;
const uint EDGEb      = 1024;
const uint MIX_PROPSb = 2048;
const uint TO_OTHERb  = 4096;
const uint LOW_VARb   = 8192;

layout (set = 0, binding = k_tilePropertiesBinding, std140)
restrict uniform TilePropertiesUB {
    // x = properties
    // yz = indices of first and last transformation rule
    uvec4 u_blockProperties[256];
    uvec4 u_wallProperties[256];

    // x = The properties that neighbors MUST have to transform
    // y = The properties that neighbors MUST NOT have to transform
    // z = Properties of the transformation
    // w = The wall that it will be transformed into
    uvec4 u_blockTransformationRules[16];
    uvec4 u_wallTransformationRules[23];
};

#endif // !TILE_PROPERTIES_UB_GLSL