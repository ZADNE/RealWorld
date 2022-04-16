R""(
in vec4 vert_light;

layout(location = 0) out vec4 light;

void main() {
	light = vert_light;
}

)""