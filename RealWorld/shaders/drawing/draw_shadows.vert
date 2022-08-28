R""(
out vec4 fColor;

layout(binding = TEX_UNIT_SHADOWS) uniform sampler2D shadowsSampler;
layout(location = 3) uniform vec2 offsetPx;
layout(location = 4) uniform ivec2 readOffsetTi;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
    ivec2 offsetTi = ivec2(gl_InstanceID % viewWidthTi, gl_InstanceID / viewWidthTi);
    vec2 posTi = POS[gl_VertexID] + vec2(offsetTi);
    
    vec2 UV = (vec2(offsetTi + readOffsetTi + LIGHT_SCALE) + 0.5) / textureSize(shadowsSampler, 0) * LIGHT_SCALE_INV;
    fColor = texture(shadowsSampler, UV);
    
    float clip = (fColor.a == 0.0) ? -1.0 : 1.0;
    gl_Position = viewMat * vec4(posTi * TILEPx - offsetPx, 0.0, clip);
    
}

)""