/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>

layout(location = 0) out        uvec4 o_tile;

void main(){
    o_tile = WOOD;
}
