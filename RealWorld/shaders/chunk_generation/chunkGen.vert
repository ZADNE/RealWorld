R""(
layout(location = 0) in vec2 position;

out vec2 pos;

void main() {
	gl_Position = chunkGenMatrix * vec4(position, 0.0, 1.0);
	pos = chunkOffsetTi + vec2(position.x, chunkDimsTi.y - position.y);
}

)""