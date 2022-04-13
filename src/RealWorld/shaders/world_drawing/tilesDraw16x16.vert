R""(
layout(location = 0) in vec2 basePosTi;
layout(location = 2) in vec2 baseUV;

out vec4 block_wall_UV;

layout(std140, binding = 0) uniform WorldDrawUniforms {
	mat4 viewsizePxMat;
	mat4 viewsizeLightingTiMat;
};

layout(std140, binding = 1) uniform ViewportMatrices {
	mat4 viewMat;
};

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 realPosPx;
uniform vec2 tileSizePx;
uniform vec4 perTileUVIncrement;
uniform int viewWidthTi;

void main() {
	vec2 posTi = basePosTi + vec2(gl_InstanceID % viewWidthTi, gl_InstanceID / viewWidthTi);
	
	vec2 globalWorldPosTi = posTi + floor(realPosPx / tileSizePx) + baseUV * vec2(-1.0, 1.0) - vec2(0.0, 1.0);

	vec2 worldTexSize = vec2(textureSize(worldTexture, 0));
	
	vec2 localWorldPosTi = mod(globalWorldPosTi, worldTexSize);
	localWorldPosTi.y = worldTexSize.y - localWorldPosTi.y - 1;
	
	vec4 tile = vec4(texelFetch(worldTexture, ivec2(localWorldPosTi), 0));
	
	//Clip if both block and wall are air
	vec4 clip = (tile.x == 0.0 && tile.z == 0.0) ? vec4(-1.0) : vec4(1.0);
	
	gl_Position = viewMat * vec4(posTi * tileSizePx - mod(realPosPx, tileSizePx), 0.0, clip);
	
	block_wall_UV = (tile.yxwz + baseUV.xyxy) * perTileUVIncrement;
}

)""