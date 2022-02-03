R""(
layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

out vec2 fragUV;

layout(std140, binding = 0) uniform GlobalMatrices{
	mat4 viewMatrix;
};

void main() {
	gl_Position.xyzw = viewMatrix * vec4(position, 0.0, 1.0);
	fragUV = UV;
}

)""