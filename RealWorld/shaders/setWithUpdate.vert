R""(
#version 460

layout(location = 0) in vec2 shiftFromCenter;
layout(location = 1) in uint setAround;

flat out uint fragSetAround;

layout(std140, binding = 0) uniform WorldUniforms {
	mat4 worldMatrix;
	vec2 worldSize;
};

layout(location = 3) uniform vec2 center;

void main() {
	vec2 pos = shiftFromCenter + vec2(center.x, worldSize.y - center.y - 1.0);
	gl_Position = worldMatrix * vec4(mod(pos, worldSize), 0.0, 1.0);
	fragSetAround = setAround;
}


)""