R""(
#version 460

layout(location = 0) out uvec4 result;

layout(location = 1) uniform uint tilesSelector;
layout(location = 2) uniform usampler2D tilesTexture[2];
layout(location = 4) uniform usampler2D materialTexture;

layout(location = 10) uniform uvec4 air;

layout(location = 33) uniform uint low;
layout(location = 34) uniform uint high;

const ivec2 offsets[] = ivec2[](
	ivec2(-1, -1), 	ivec2(+0, -1), 	ivec2(+1, -1),
	ivec2(-1, +0), 				 	ivec2(+1, +0),
	ivec2(-1, +1), 	ivec2(+0, +1), 	ivec2(+1, +1)
);

void main() {
	ivec2 pos = ivec2(gl_FragCoord.xy);
	uint tilesIndex = tilesSelector % 2;
	uvec4 previous = texelFetch(tilesTexture[tilesIndex], pos, 0);
	uvec4 material = texelFetch(materialTexture, pos, 0);
	
	//Check neighbors
	uvec2 neighborsN = uvec2(0, 0);
	for (int i = 0; i < offsets.length(); i++){
		uvec4 neighbor = texelFetch(tilesTexture[tilesIndex], pos + offsets[i], 0);
		neighborsN += mix(uvec2(0, 0), uvec2(1, 1), notEqual(neighbor.rb, air.rb));
	}
	
	if (previous.g > 0){//If there are more cycles to be done on this tile
		previous.g -= 1;
		uvec4 resultMaterial = uvec4(material.r, previous.gba);
		uvec4 resultAir = uvec4(air.r, previous.gba);
		
		if (previous.r == air.x){
			result = neighborsN.x > high ? resultMaterial : previous;
		} else {
			result = neighborsN.x < low ? resultAir : previous;
		}
	} else {//No more cycles to be done on this cell
		result = previous;
	}
}

)""