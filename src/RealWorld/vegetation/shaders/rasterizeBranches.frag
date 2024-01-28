/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
const int BranchSB_BINDING = 0;
#include <RealWorld/vegetation/shaders/BranchSB.glsl>

layout(input_attachment_index = 0, set = 0, binding = 1)
uniform usubpassInput u_worldTexSI;

layout (location = 0) out uvec4    o_tile;

layout (location = 0) in vec2      i_uv;
layout (location = 1) in flat uint i_branchIndex15wallType31;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    if (isNonsolidWall(prevTile.WL_T)){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        uint variant = loadBranchTexel(branchIndex, ivec2(round(i_uv)));
        uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
        o_tile = uvec4(prevTile.BL, wallType, variant);
    } else {
        o_tile = prevTile;
    }
}
