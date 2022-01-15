R""(
#version 440

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

out vec2 blockUV;
out vec2 wallUV;

layout(std140) uniform WorldDrawUniforms {
	mat4 viewMat;
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
};

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 positionPx;
uniform ivec2 blockSizePx;
uniform ivec2 lastBlock_lastVar;

void main() {
	gl_Position = viewMat * vec4(position - mod(positionPx, blockSizePx), 0.0, 1.0);
	vec2 worldUV = floor((positionPx + position - UV * blockSizePx * lastBlock_lastVar) / blockSizePx);
	ivec2 textureUV = ivec2(mod(worldUV - vec2(0.0, 1.0), vec2(textureSize(worldTexture, 0)))) ;
	uvec4 data = texelFetch(worldTexture, ivec2(textureUV.x, textureSize(worldTexture, 0).y - textureUV.y - 1), 0);
	blockUV = vec2(UV.x + float(data.y) / float(lastBlock_lastVar.x) , UV.y + float(data.x) / float(-lastBlock_lastVar.y));
	wallUV = vec2(UV.x + float(data.w) / float(lastBlock_lastVar.x) , UV.y + float(data.z) / float(-lastBlock_lastVar.y));
}

)""