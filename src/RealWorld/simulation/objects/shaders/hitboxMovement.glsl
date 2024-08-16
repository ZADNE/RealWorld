/*!
 *  @author     Dubsky Tomas
 */
#ifndef HITBOX_MOVEMENT_GLSL
#define HITBOX_MOVEMENT_GLSL

#include <RealWorld/constants/tile.glsl>
#include <RealWorld/constants/world.glsl>
#include <RealWorld/simulation/tiles/shaders/tileLoad.glsl>

uint ground(vec2 posPx, vec2 dimsPx, ivec2 worldTexMaskTi) {
    uint grnd = k_airBl;
    int  yTi  = int(pxToTi(posPx.y - 1.0));
    for (int xTi = int(pxToTi(posPx.x)); xTi <= pxToTi(posPx.x + dimsPx.x); ++xTi) {
        ivec2 posAt = tiToAt(ivec2(xTi, yTi), worldTexMaskTi);
        grnd = min(grnd, loadBlock(posAt).L_T);
    }
    return grnd;
}

bool overlapsBlocks(vec2 posPx, vec2 dimsPx, ivec2 worldTexMaskTi) {
    for (int yTi = int(pxToTi(posPx.y)); yTi <= pxToTi(posPx.y + dimsPx.y); ++yTi) {
        for (int xTi = int(pxToTi(posPx.x)); xTi <= pxToTi(posPx.x + dimsPx.x); ++xTi) {
            ivec2 posAt = tiToAt(ivec2(xTi, yTi), worldTexMaskTi);
            if (isSolidBlock(loadBlock(posAt).L_T)) {
                return true;
            }
        }
    }
    return false;
}

void moveFinely(inout vec2 posPx, vec2 dimsPx, inout vec2 velPx, ivec2 worldTexMaskTi){
    vec2 truncVelPx = trunc(velPx);
    // Move in X direction
    if (overlapsBlocks(vec2(posPx.x + velPx.x, posPx.y), dimsPx, worldTexMaskTi)) {
        float velSign = sign(velPx.x);
        while (!overlapsBlocks(vec2(posPx.x + velSign, posPx.y), dimsPx, worldTexMaskTi)) {
            posPx.x += velSign;
        }
        velPx.x = 0.0;
    } else {
        posPx.x += truncVelPx.x;
    }

    // Move in Y direction
    if (overlapsBlocks(vec2(posPx.x, posPx.y + velPx.y), dimsPx, worldTexMaskTi)) {
        float velSign = sign(velPx.y);
        while (!overlapsBlocks(vec2(posPx.x, posPx.y + velSign), dimsPx, worldTexMaskTi)) {
            posPx.y += velSign;
        }
        velPx.y = 0.0;
    } else {
        posPx.y += truncVelPx.y;
    }
}

bool overlapsPoint(vec2 posPx, vec2 dimsPx, vec2 pointPx) {
    return all(lessThan(posPx, pointPx)) && all(greaterThanEqual(posPx + dimsPx, pointPx));
}

#endif // !HITBOX_MOVEMENT_GLSL
