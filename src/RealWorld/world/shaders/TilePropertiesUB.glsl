/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_PROPERTIES_UB_GLSL
#define TILE_PROPERTIES_UB_GLSL

//Tile properties
const uint STONEb =     1 << 0;
const uint DIRTb =      1 << 1;
const uint GRASSb =     1 << 2;
const uint BURNINGb =   1 << 3;
const uint HALLOWb =    1 << 4;

//Wall properties
const uint EDGEb =      1 << 0;
const uint MIX_PROPSb = 1 << 1;//Mix properties with the other layer 

layout(set = 0, binding = TilePropertiesUB_BINDING, std140)
restrict uniform TilePropertiesUB {
    //x = properties
    //yz = indices of first and last transformation rule
    uvec4 u_blockProperties[256];
    uvec4 u_wallProperties[256];

    //x = The properties that neighbors MUST have to transform
    //y = The properties that neighbors MUST NOT have to transform
    //z = Properties of the transformation
    //w = The wall that it will be transformed into
    uvec4 u_blockTransformationRules[16];
    uvec4 u_wallTransformationRules[16];
};

#endif // TILE_PROPERTIES_UB_GLSL