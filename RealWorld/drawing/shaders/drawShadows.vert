#version 460
#include <RealWorld/reserved_units/textures.glsl>
#include <RealWorld/constants/light.glsl>
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout(location = 0) out vec4 shadowColor;

layout(binding = TEX_UNIT_SHADOWS) uniform sampler2D shadowsSampler;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
    ivec2 offsetTi = ivec2(gl_InstanceIndex % viewWidthTi, gl_InstanceIndex / viewWidthTi);
    vec2 posTi = POS[gl_VertexIndex] + vec2(offsetTi);
    
    vec2 UV = (vec2(offsetTi + drawShadowsReadOffsetTi + LIGHT_SCALE) + 0.5) / textureSize(shadowsSampler, 0) * LIGHT_SCALE_INV;
    shadowColor = texture(shadowsSampler, UV);
    
    float clip = (shadowColor.a == 0.0) ? -1.0 : 1.0;
    gl_Position = viewMat * vec4(posTi * TILEPx - botLeftPxModTilePx, 0.0, clip);
    
}
