/**
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/Tile.glsl>
#include <RealWorld/constants/World.glsl>
#include <RealWorld/simulation/vegetation/shaders/branchRasterizationPll.glsl>
#include <RealWorld/simulation/vegetation/shaders/BranchSB.glsl>
#include <RealWorld/simulation/tiles/shaders/tileLoadStore.glsl>
#include <RealWorld/simulation/vegetation/shaders/VegDynamicsPC.glsl>

layout(input_attachment_index = 0, set = 0, binding = k_wallLayerAttBinding)
uniform usubpassInput u_wallLayerSI;

layout (location = 0) out uvec2    o_wall;

layout (location = 0) in vec2      i_uv;
layout (location = 1) in vec2      i_tipDir;
layout (location = 2) in vec2      i_sizeTi;
layout (location = 3) in flat uint i_branchIndex15wallType31;

/// @return Wall type of the wood
uint woodBasicStateToWall(uint basicState, uint naturalWall){
    switch (basicState) {
    case k_woodBasicStateBurning:   return k_burningWoodWl;
    case k_woodBasicStateBurnt:     return k_burntWoodWl;
    case k_woodBasicStateHallow:    return k_hallowWoodWl;
    case k_woodBasicStateRemoved:   return k_airWl;
    default:                        return naturalWall;
    }
}

void main(){
    uvec2 prevWall = subpassLoad(u_wallLayerSI).rg;
    if (isNonsolidWall(prevWall.L_T)){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        vec2 uv = ivec2(round(i_uv * (i_sizeTi - 1.0)));
        uvec2 texel = loadBranchTexel(branchIndex, ivec2(uv));
        uint wallType = woodBasicStateToWall(
            texel.L_T,
            (i_branchIndex15wallType31 >> 16) & 0xff
        );
        o_wall = uvec2(wallType, texel.L_V);
        uint bud = (texel.L_V & 7);
        if (isWoodWall(wallType) && bud > 0){ // If should spawn root leaf
            uv /= i_sizeTi;
            vec2 normal = vec2(i_tipDir.y, -i_tipDir.x);
            vec2 dir = (uv.x - 0.5) * normal + (uv.y * 0.5) * i_tipDir;
            ivec2 leafPosAt = tiToAt(ivec2(gl_FragCoord.xy + dir * float(bud * 3)), p_.worldTexMaskTi);
            uvec2 leaf = loadWall(leafPosAt);
            uint leafType = wallType + k_woodToLeafTypeOffset;
            if (leaf.L_T == k_airWl){
                storeWall(leafPosAt, uvec2(leafType, 8));
            } else if (leaf.L_T == leafType){
                uint var = min(leaf.L_V + 6, 8 + bud * 6);
                storeWall(leafPosAt, uvec2(leafType, var));
            }
        }
    } else {
        o_wall = prevWall;
    }
}
