R""(
#version 460

in vec2 pos;

out uvec4 result;

layout(location = 2) uniform usampler2D worldTexture;
layout(location = 10) uniform uvec2 air;

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
	
vec2 hash22(vec2 p){
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}	
	
void main() {
	ivec2 posBc = ivec2(gl_FragCoord.xy);
	uvec2 block_wall = texelFetch(worldTexture, posBc, 0).rb;

	//Fetching surrounds
	uvec2 toptop = 	uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(+0, -1), 0).rb, air)) << 7;
	uvec2 topright = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(+1, -1), 0).rb, air)) << 6;
	uvec2 midright = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(+1, +0), 0).rb, air)) << 5;
	uvec2 botright = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(+1, +1), 0).rb, air)) << 4;
	uvec2 botbot = uvec2(notEqual(texelFetch(worldTexture, posBc + 		ivec2(+0, +1), 0).rb, air)) << 3;
	uvec2 botleft = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(-1, +1), 0).rb, air)) << 2;
	uvec2 midleft = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(-1, +0), 0).rb, air)) << 1;
	uvec2 topleft = uvec2(notEqual(texelFetch(worldTexture, posBc + 	ivec2(-1, -1), 0).rb, air));
	
	
	uvec2 varIndex = toptop | topright | midright | botright | botbot | botleft | midleft | topleft;
	
	uvec2 varSet = uvec2(hash22(posBc) * 4.0) * 47;
	
	result = uvec4(block_wall.x, variations[varIndex.x] + varSet.x, block_wall.y, variations[varIndex.y] + varSet.y);

}

)""