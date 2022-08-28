R""(
layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

out vec2 vUV;

layout(std140, binding = 0) uniform GlobalMatrices{
    mat4 viewMatrix;
};

void main() {
    gl_Position = viewMatrix * vec4(position, 0.0, 1.0);
    vUV = UV;
}

)""