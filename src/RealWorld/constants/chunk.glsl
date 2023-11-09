/*! 
 *  @author    Dubsky Tomas
 */
#ifndef CHUNK_GLSL
#define CHUNK_GLSL

const uvec2 uChunkTi = {128, 128};
const ivec2 iChunkTi = ivec2(uChunkTi);
const vec2  ChunkTi  = vec2(uChunkTi);
const ivec2 k_chunkLowZeroBits = {7, 7};

vec2  tiToCh(vec2 posTi)  { return floor(posTi / ChunkTi); }
float tiToCh(float posTi) { return floor(posTi / ChunkTi.x); }
ivec2 tiToCh(ivec2 posTi) { return posTi >> k_chunkLowZeroBits; }
int   tiToCh(int posTi)   { return posTi >> k_chunkLowZeroBits.x; }

vec2  chToTi(vec2 posCh)  { return posCh * ChunkTi; }
float chToTi(float posCh) { return posCh * ChunkTi.x; }
ivec2 chToTi(ivec2 posCh) { return posCh << k_chunkLowZeroBits; }
int   chToTi(int posCh)   { return posCh << k_chunkLowZeroBits.x; }

#endif // !CHUNK_GLSL