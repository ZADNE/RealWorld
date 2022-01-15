R""(
#version 440

in vec4 fragColour;
in vec2 fragUV;

layout(location = 1) out vec4 colour;

layout(location = 1) uniform sampler2D baseTexture;

void main() {
	colour = fragColour * texture(baseTexture, fragUV);
}

)""