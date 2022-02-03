R""(
in vec2 fragUV;

out vec4 colour;

layout(location = 1) uniform sampler2D baseTexture;

void main() {;
	colour = texture(baseTexture, fragUV);
}
	
)""