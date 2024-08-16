/*!
 *  @author     Dubsky Tomas
 */
#ifndef DROPPED_TILES_SIM_PC_GLSL
#define DROPPED_TILES_SIM_PC_GLSL

layout (push_constant, std430)
restrict uniform DroppedTilesSimPC {
    vec2  p_playerBotLeftPx;
    vec2  p_playerDimsPx;
    ivec2 p_worldTexMaskTi;
    float p_timeSec;
};

#endif // !DROPPED_TILES_SIM_PC_GLSL