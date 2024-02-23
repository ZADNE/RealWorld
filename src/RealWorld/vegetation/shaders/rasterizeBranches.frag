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

layout (location = 0) in vec2      i_uv;
layout (location = 1) in vec2      i_tipDir;
layout (location = 2) in vec2      i_sizeTi;
layout (location = 3) in flat uint i_branchIndex15wallType31;

void main(){
    uvec4 prevTile = subpassLoad(u_worldTexSI);
    if (isNonsolidWall(prevTile.WL_T)){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        vec2 uv = ivec2(round(i_uv * (i_sizeTi - 1.0)));
        uint variant = loadBranchTexel(branchIndex, ivec2(uv));
        uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
        o_tile = uvec4(prevTile.BL, wallType, variant);
        uint bud = (variant & 7);
        if (bud > 0){ // If should spawn root leaf
            uv /= i_sizeTi;
            vec2 normal = vec2(i_tipDir.y, -i_tipDir.x);
            vec2 dir = (uv.x - 0.5) * normal + (uv.y * 0.5) * i_tipDir;
            ivec2 leafPosIm = imPos(ivec2(gl_FragCoord.xy + dir * float(bud * 3)));
            uvec4 leaf = tileLoadIm(leafPosIm);
            if (leaf.WL_T == k_airWl){
                tileStoreIm(leafPosIm, uvec4(leaf.BL, k_leafWl, 8));
            } else if (leaf.WL_T == k_leafWl){
                uint var = min(leaf.WL_V + 6, 8 + bud * 6);
                tileStoreIm(leafPosIm, uvec4(leaf.BL, k_leafWl, var));
            }
        }
    } else {
        o_tile = prevTile;
    }
}
