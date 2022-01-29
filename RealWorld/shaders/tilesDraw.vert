R""(
#version 460

layout(location = 0) in vec2 basePosBc;
layout(location = 2) in vec2 baseUV;

out vec4 block_wall_UV;

layout(std140, binding = 0) uniform WorldDrawUniforms {
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
};

layout(std140, binding = 1) uniform ViewportMatrices {
	mat4 viewMat;
};

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 realPosPx;
uniform vec2 blockSizePx;
uniform vec2 perTileUVIncrement;
uniform int viewWidthBc;

void main() {
	vec2 posBc = basePosBc + vec2(gl_InstanceID % viewWidthBc, gl_InstanceID / viewWidthBc);
	
	vec2 globalWorldPosBc = posBc + floor(realPosPx / blockSizePx) + baseUV * vec2(-1.0, 1.0) - vec2(0.0, 1.0);

	vec2 worldTexSize = vec2(textureSize(worldTexture, 0));
	
	vec2 localWorldPosBc = mod(globalWorldPosBc, worldTexSize);
	localWorldPosBc.y = worldTexSize.y - localWorldPosBc.y - 1;
	
	vec4 tile = vec4(texelFetch(worldTexture, ivec2(localWorldPosBc), 0));
	
	//Clip if both block and wall are air
	vec4 clip = (tile.x == 0.0 && tile.z == 0.0) ? vec4(-1.0) : vec4(1.0);
	
	gl_Position = viewMat * vec4(posBc * blockSizePx - mod(realPosPx, blockSizePx), 0.0, clip);
	
	block_wall_UV = (tile.yxwz + baseUV.xyxy) * perTileUVIncrement.xyxy;
}

)""