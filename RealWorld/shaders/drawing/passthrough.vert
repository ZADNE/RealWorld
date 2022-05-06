R""(
layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

void main() {
	gl_Position = viewsizeLightingUnMat * vec4(position, 0.0, 1.0);
}

)""