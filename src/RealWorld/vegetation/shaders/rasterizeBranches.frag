/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
const uint worldImage_BINDING = 2;
#include <RealWorld/world/shaders/tileLoadStore.glsl>
const uint BranchRasterSB_BINDING = 3;
#include <RealWorld/vegetation/shaders/BranchRasterSB.glsl>

layout (location = 0) in vec2      i_uv;
layout (location = 1) in flat uint i_branchIndex0parentDiscr16wallType24;

void main(){
    ivec2 posIm = imPos(ivec2(gl_FragCoord.xy) - (imageSize(u_worldImage) >> 1));
    uvec4 prevTile = tileLoadIm(posIm);
    if (isAirWall(prevTile.WL_T)){
        uint branchIndex = i_branchIndex0parentDiscr16wallType24 & 0xffff;
        uint variant = branchTexel(branchIndex, ivec2(round(i_uv)));
        uint parentDiscr = (i_branchIndex0parentDiscr16wallType24 >> 16) & 0xff;
        uint wallType = (i_branchIndex0parentDiscr16wallType24 >> 24) & 0xff;
        tileStoreIm(posIm, uvec4(prevTile.BL_T, parentDiscr, wallType, variant));
    }
}
