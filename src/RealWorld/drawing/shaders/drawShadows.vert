/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/constants/light.glsl>
#include <RealWorld/drawing/shaders/ShadowDrawingPC.glsl>

layout(location = 0) out vec4 o_shadowColor;

layout(set = 0, binding = 0) uniform sampler2D u_shadowsSampler;

const vec2 k_pos[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
    ivec2 offsetTi = ivec2(gl_InstanceIndex % p_viewSizeTi.x, gl_InstanceIndex / p_viewSizeTi.x);
    vec2 posTi = k_pos[gl_VertexIndex] + vec2(offsetTi);

    vec2 UV = (vec2(offsetTi + p_readOffsetTi + k_iLightScale) + 0.5) / textureSize(u_shadowsSampler, 0) * k_iLightScaleInv;
    o_shadowColor = texture(u_shadowsSampler, UV);

    float clip = (o_shadowColor.a == 0.0) ? -1.0 : 1.0;
    gl_Position = p_viewMat * vec4(posTi * TilePx - p_botLeftPxModTilePx, 0.0, clip);
}