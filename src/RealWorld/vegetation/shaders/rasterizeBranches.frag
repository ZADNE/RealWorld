/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
const uint BranchRasterSB_BINDING = 3;
#include <RealWorld/vegetation/shaders/BranchRasterSB.glsl>

layout(input_attachment_index = 0, set = 0, binding = 2)
uniform usubpassInput u_worldTexSI;

layout (location = 0) out uvec4    o_tile;

layout (location = 0) in vec2      i_uv;
layout (location = 1) in flat uint i_branchIndex0wallType16;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    if (isNonsolidWall(prevTile.WL_T)){
        uint branchIndex = i_branchIndex0wallType16 & 0xffff;
        uint variant = loadBranchTexel(branchIndex, ivec2(round(i_uv)));
        uint wallType = (i_branchIndex0wallType16 >> 16) & 0xff;
        o_tile = uvec4(prevTile.BL, wallType, variant);
    } else {
        o_tile = prevTile;
    }
}
