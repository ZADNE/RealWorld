/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>

layout(location = 0) out        uvec4 o_tile;

layout(input_attachment_index = 0, set = 0, binding = 2)
uniform usubpassInput u_worldTexSI;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    if (prevTile.TL_T == AIR.TL_T){
        o_tile = WOOD;
    } else {
        o_tile = prevTile;
    }
}
