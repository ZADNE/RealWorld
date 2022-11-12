#version 460

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

layout(location = 0) out vec2 minimapUV;

layout(std140, binding = 0) uniform GlobalMatrices{
    mat4 viewMatrix;
};

void main() {
    gl_Position = viewMatrix * vec4(position, 0.0, 1.0);
    minimapUV = UV;
}