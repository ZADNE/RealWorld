R""(
flat in uint fragSetAround;

out uvec4 result;

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 10) uniform uvec2 air;
layout(location = 5) uniform uint set;
layout(location = 6) uniform float time;

const uint variations[256] = uint[256](
	29, 29, 12, 12, 29, 29, 12, 12,//  0
	24, 24, 38, 38, 24, 24,  4,  4,//  8
	29, 29, 12, 12, 29, 29, 12, 12,// 16
	24, 24, 38, 38, 24, 24,  4,  4,// 24
	27, 27,  9,  9, 27, 27,  9,  9,// 32
	32, 32, 43, 43, 32, 32, 17, 17,// 40
	27, 27,  9,  9, 27, 27,  9,  9,// 48
	22, 22, 35, 35, 22, 22,  1,  1,// 56
	29, 29, 12, 12, 29, 29, 12, 12,// 64
	24, 24, 38, 38, 24, 24,  4,  4,// 72
	29, 29, 12, 12, 29, 29, 12, 12,// 80
	24, 24, 38, 38, 24, 24,  4,  4,// 88
	27, 27,  9,  9, 27, 27,  9,  9,// 96
	32, 32, 43, 43, 32, 32, 17, 17,//104
	27, 27,  9,  9, 27, 27,  9,  9,//112
	22, 22, 35, 35, 22, 22,  1,  1,//120
	28, 28, 13, 11, 28, 28, 13, 11,//128
	23, 23, 39, 37, 23, 23,  5,  3,//136
	28, 28, 13, 11, 28, 28, 13, 11,//144
	23, 23, 39, 37, 23, 23,  5,  3,//152
	30, 30, 15, 14, 30, 30, 15, 14,//160
	33, 33, 46, 45, 33, 33, 20, 19,//168
	30, 30, 15, 14, 30, 30, 15, 14,//176
	25, 25, 41, 40, 25, 25,  7,  6,//184
	28, 28, 13, 11, 28, 28, 13, 11,//192
	23, 23, 39, 37, 23, 23,  5,  3,//200
	28, 28, 13, 11, 28, 28, 13, 11,//208
	23, 23, 39, 37, 23, 23,  5,  3,//216
	26, 26, 10,  8, 26, 26, 10,  8,//224
	31, 31, 44, 42, 31, 31, 18, 16,//232
	26, 26, 10,  8, 26, 26, 10,  8,//240
	21, 21, 36, 34, 21, 21,  2,  0);//248
	
vec2 hash23(vec3 p3){
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

uvec4 worldTexelFetchModulo(ivec2 pos){
	return texelFetch(worldTexture, ivec2(mod(pos, textureSize(worldTexture, 0))), 0);
}

void main() {
	ivec2 posBc = ivec2(gl_FragCoord.xy);
	uvec4 prev = texelFetch(worldTexture, posBc, 0);
	
	bool settingThis = fragSetAround == 0;
	bvec2 settingBlockHere = bvec2(ivec2(settingThis, settingThis) * ivec2(set != air));
	
	uvec4 block_wall = settingThis ? uvec4(set, prev.g, set, prev.a) : prev;
	uvec2 varSet = uvec2(hash23(vec3(posBc, time)) * 4.0) * 47;
	block_wall.ga = mix((block_wall.ga / 47) * 47, varSet, settingBlockHere);
	
	//Fetching surrounds
	uvec2 toptop = 	uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(0, -1)).rb, air)) << 7;
	uvec2 topright = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(1, -1)).rb, air)) << 6;
	uvec2 midright = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(1, 0)).rb, air)) << 5;
	uvec2 botright = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(1, 1)).rb, air)) << 4;
	uvec2 botbot = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(0, 1)).rb, air)) << 3;
	uvec2 botleft = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(-1, 1)).rb, air)) << 2;
	uvec2 midleft = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(-1, 0)).rb, air)) << 1;
	uvec2 topleft = uvec2(notEqual(worldTexelFetchModulo(posBc + ivec2(-1, -1)).rb, air));
	
	
	uvec2 varIndex = toptop | topright | midright | botright | botbot | botleft | midleft | topleft;
	
	//Adding or removing?
	varIndex = (set == air) ? (varIndex & uvec2(~fragSetAround, ~fragSetAround)) : (varIndex | uvec2(fragSetAround, fragSetAround));
	
	result = block_wall + uvec4(0, variations[varIndex.x], 0, variations[varIndex.y]);
}

)""