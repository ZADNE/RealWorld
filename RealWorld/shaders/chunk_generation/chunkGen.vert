R""(
layout(location = 0) in vec2 positionTi;

out vec2 posTi;

void main() {
	gl_Position = chunkGenMatrix * vec4(positionTi, 0.0, 1.0);
	posTi = chunkOffsetTi + vec2(positionTi.x, chunkDimsTi.y - positionTi.y);
}

)""