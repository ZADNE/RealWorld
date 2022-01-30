R""(
#version 460

in vec4 vert_light;
in vec4 vert_diaphragm;

layout(location = 0) out vec4 diaphragm;
layout(location = 1) out vec4 light;

void main() {
	diaphragm = vert_diaphragm;
	light = vert_light;
}

)""