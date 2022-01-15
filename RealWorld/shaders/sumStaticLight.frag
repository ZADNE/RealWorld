R""(
#version 440

in vec4 fragColour;
in float lightDir;
in float lightCone;

layout(location = 0) out vec4 diaphragm;

void main() {
	diaphragm = vec4(0.0, 0.0, 0.0, fragColour.a);
}

)""