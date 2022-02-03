R""(
layout(location = 0) out vec4 diaphragm;
layout(location = 1) out vec4 light;

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 botLeftBc;

layout(location = 4) uniform vec4 daylight;
layout(location = 5) uniform vec4 diaphragms;//air, wall, block, not-used-yet

void main() {
	uvec4 texel = texelFetch(worldTexture, ivec2(mod(botLeftBc + gl_FragCoord.xy, vec2(textureSize(worldTexture, 0)))), 0);
	if (texel.r == 0u){//If there is air-block
		if (texel.b == 0u){//If there is air-wall
			diaphragm = vec4(diaphragms.x, 0.0, 1.0, 0.0);
			light = daylight;
		} else {//There is not a block but there is a wall
			diaphragm = vec4(diaphragms.y, 0.0, 0.0, 0.0);
			light = vec4(0.0, 0.0, 0.0, 0.0);
		}
	} else {//There is a block
		diaphragm = vec4(diaphragms.z, 0.0, 0.0, 0.0);
		light = vec4(0.0, 0.0, 0.0, 0.0);
	}
}

)""