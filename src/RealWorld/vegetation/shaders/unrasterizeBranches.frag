/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/vegetation/shaders/branchRasterizationPll.glsl>
#include <RealWorld/vegetation/shaders/BranchSB.glsl>

layout(input_attachment_index = 0, set = 0, binding = k_wallLayerAttBinding)
uniform usubpassInput u_wallLayerSI;

layout (location = 0) out uvec2    o_wall;

layout (location = 0) in vec2      i_uv;
layout (location = 2) in vec2      i_sizeTi;
layout (location = 3) in flat uint i_branchIndex15wallType31;

void main(){
    uvec2 prevWall = subpassLoad(u_wallLayerSI).rg;
    uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
    if (prevWall.L_T == wallType){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        ivec2 uv = ivec2(round(i_uv * (i_sizeTi - 1.0)));
        //storeBranchTexel(branchIndex, uv, prevWall.L_V);
        o_wall = uvec2(k_airWl, 0);
    } else {
        o_wall = prevWall;
    }
}
