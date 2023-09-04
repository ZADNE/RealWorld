/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
const uint worldImage_BINDING = 2;
#include <RealWorld/world/shaders/tileLoadStore.glsl>

void main(){
    ivec2 posIm = imPos(ivec2(gl_FragCoord.xy) - (imageSize(u_worldImage) >> 1));
    uvec4 prevTile = tileLoadIm(posIm);
    if (prevTile.WL_T == WOOD.WL_T){
        tileStoreIm(posIm, uvec4(prevTile.BL, AIR.WL));
    }
}
