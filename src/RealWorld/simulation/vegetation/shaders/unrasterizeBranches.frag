/**
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/Tile.glsl>
#include <RealWorld/simulation/vegetation/shaders/branchRasterizationPll.glsl>
#include <RealWorld/simulation/vegetation/shaders/BranchSB.glsl>

layout(input_attachment_index = 0, set = 0, binding = k_wallLayerAttBinding)
uniform usubpassInput u_wallLayerSI;

layout (location = 0) out uvec2    o_wall;

layout (location = 0) in vec2      i_uv;
layout (location = 2) in vec2      i_sizeTi;
layout (location = 3) in flat uint i_branchIndex15wallType31;

/// @return Basic state of the wood if prevWall is related, ~0 otherwise
uint wallToWoodBasicState(uint prevWall, uint naturalWall){
    if (prevWall == naturalWall) {
        return k_woodBasicStateNatural;
    } else if (prevWall == (naturalWall | k_looseTypeBit)){
        return k_woodBasicStateRemoved;
    }
    switch (prevWall) {
    case k_burningWoodWl: return k_woodBasicStateBurning;
    case k_burntWoodWl:   return k_woodBasicStateBurnt;
    case k_hallowWoodWl:  return k_woodBasicStateHallow;
    }
    return ~0;
}

void main(){
    uvec2 prevWall = subpassLoad(u_wallLayerSI).rg;
    uint wallType = (i_branchIndex15wallType31 >> 16) & 0xff;
    uint basicState = wallToWoodBasicState(prevWall.L_T, wallType);
    if (basicState != ~0){
        uint branchIndex = i_branchIndex15wallType31 & 0xffff;
        ivec2 uv = ivec2(round(i_uv * (i_sizeTi - 1.0)));
        storeBranchBasicState(branchIndex, uv, basicState);
        o_wall = uvec2(k_airWl, 0);
    } else {
        o_wall = prevWall;
    }
}
