R""(
#version 440

in vec2 fragUV;

out vec4 colour;

layout(location = 1) uniform usampler2D baseTexture;

void main() {
	uvec4 tex = texture(baseTexture, fragUV);
	if (tex.r == 0) {//No block
		if (tex.b == 1) {//Stone wall
			colour = vec4(0.8, 0.8, 0.8, 1.0);
		} else if (tex.b == 3) {//Dirt wall
			colour = vec4(0.8, 0.2, 0.2, 1.0);
		} else if (tex.b == 4) {//Leaves
			colour = vec4(0.0, 0.0, 0.0, 1.0);
		} else {
			colour = vec4(0.1, 0.1, 0.9, 1.0);
		}
	} else {
		if (tex.r == 1) {//Stone block
			colour = vec4(0.5, 0.5, 0.5, 1.0);
		} else if (tex.r == 2) {//Dirt block
			colour = vec4(0.8, 0.2, 0.2, 1.0);
		} else if (tex.r == 3) {//Grass block
			colour = vec4(0.1, 1.0, 0.1, 1.0);
		} else {
			colour = vec4(1.0, 0.0, 0.0, 1.0);
		}
	}
}
)""