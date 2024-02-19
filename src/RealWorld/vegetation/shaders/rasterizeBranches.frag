/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/vegetation/shaders/branchRasterizationPll.glsl>
#include <RealWorld/vegetation/shaders/BranchSB.glsl>
#include <RealWorld/world/shaders/tileLoadStore.glsl>

layout(input_attachment_index = 0, set = 0, binding = k_worldTexAttBinding)
uniform usubpassInput u_worldTexSI;

layout (location = 0) out uvec4     o_tile;

layout (location = 0) in vec2       i_uv;
layout (location = 1) in vec2       i_normal;
layout (location = 2) in flat uint  i_branchIndex15wallType31;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    if (isNonsolidWall(prevTile.WL_T)){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        uint variant = loadBranchTexel(branchIndex, ivec2(round(i_uv)));
        uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
        o_tile = uvec4(prevTile.BL, wallType, variant);
        uint bud = (variant & 3);
        if (bud > 0){ // If should spawn root leaf
            ivec2 leafPosIm = imPos(ivec2(gl_FragCoord.xy + i_normal * float(bud * 4)));
            uvec4 leaf = tileLoadIm(leafPosIm);
            if (isNonsolidWall(leaf.WL_T)){
                tileStoreIm(leafPosIm, uvec4(leaf.BL, k_leafWl, 2));
            }
        }
    } else {
        o_tile = prevTile;
    }
}
