/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_CHUNK_GLSL
#define RW_CHUNK_GLSL
#include <RealShaders/CppIntegration.glsl>

const uvec2 uChunkTi = {128, 128};
const ivec2 iChunkTi = ivec2(uChunkTi);
const vec2  ChunkTi  = vec2(uChunkTi);
const ivec2 k_chunkLowZeroBits = {7, 7};

inline vec2  tiToCh(vec2 posTi)  { return floor(posTi / ChunkTi); }
inline float tiToCh(float posTi) { return floor(posTi / ChunkTi.x); }
inline ivec2 tiToCh(ivec2 posTi) { return posTi >> k_chunkLowZeroBits; }
inline int   tiToCh(int posTi)   { return posTi >> k_chunkLowZeroBits.x; }

inline vec2  chToTi(vec2 posCh)  { return posCh * ChunkTi; }
inline float chToTi(float posCh) { return posCh * ChunkTi.x; }
inline ivec2 chToTi(ivec2 posCh) { return posCh << k_chunkLowZeroBits; }
inline int   chToTi(int posCh)   { return posCh << k_chunkLowZeroBits.x; }

#endif // !RW_CHUNK_GLSL