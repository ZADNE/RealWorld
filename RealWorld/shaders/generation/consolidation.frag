R""(
#line 3
layout(location = 0) out uvec4 result;

layout(binding = TEX_UNIT_GEN_TILES0) uniform usampler2D tilesTexture[2];
layout(binding = TEX_UNIT_GEN_MATERIAL) uniform usampler2D materialTexture;

layout(location = 33) uniform ivec2 thresholds;
const int LOW = 0;
const int HIGH = 1;
layout(location = 34) uniform uint cycleN;

const ivec2 offsets[] = ivec2[](
    ivec2(-1, -1),     ivec2(+0, -1),   ivec2(+1, -1),
    ivec2(-1, +0),                      ivec2(+1, +0),
    ivec2(-1, +1),     ivec2(+0, +1),   ivec2(+1, +1)
);


void main() {
    const uint read = cycleN % 2;

    ivec2 pos = ivec2(gl_FragCoord.xy);
    uvec4 previous = texelFetch(tilesTexture[read], pos, 0);
    uvec4 material = texelFetch(materialTexture, pos, 0);
    
    //Check neighbors
    int neighborsN = 0;
    for (int i = 0; i < offsets.length(); i++){
        neighborsN += int(!isAirBlock(texelFetch(tilesTexture[read], pos + offsets[i], 0).BL_T));
    }
    
    if (previous.g > 0){//If there are more cycles to be done on this tile
        previous.g -= 1;
        uvec4 resultMaterial = uvec4(material.r, previous.gba);
        uvec4 resultAir = uvec4(AIR.r, previous.gba);
        
        if (previous.BL_T == AIR_BL){
            result = neighborsN > thresholds[HIGH] ? resultMaterial : previous;
        } else {
            result = neighborsN < thresholds[LOW] ? resultAir : previous;
        }
    } else {//No more cycles to be done on this cell
        result = previous;
    }
}

)""