/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
const uint BranchSB_BINDING = 0;
#include <RealWorld/vegetation/shaders/BranchSB.glsl>

layout(input_attachment_index = 0, set = 0, binding = 1)
uniform usubpassInput u_worldTexSI;

layout (location = 0) out uvec4    o_tile;

layout (location = 0) in vec2      i_uv;
layout (location = 1) in flat uint i_branchIndex15wallType31;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
    if (prevTile.WL_T == wallType){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        storeBranchTexel(branchIndex, ivec2(round(i_uv)), prevTile.WL_V);
        o_tile = uvec4(prevTile.BL, AIR.WL);
    } else {
        o_tile = prevTile;
    }
}
