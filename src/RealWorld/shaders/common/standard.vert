R""(
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 UV;

out vec4 fragColour;
out vec2 fragUV;

layout(location = 0) uniform mat4 projectionMatrix;

void main() {
	gl_Position = projectionMatrix * vec4(position, 0.0, 1.0);
	fragColour = colour;
	fragUV = vec2(UV.x, 1.0 - UV.y);
}
)""