/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_WORLEY_VORONOI_NOISE_GLSL
#define RW_WORLEY_VORONOI_NOISE_GLSL
#include <RealWorld/generation/external_shaders/float_hash.glsl>

/**
 * @brief   Simultaneously calculates Worley and Voronoi noise
 * @details The grid is unit-sized
 * @return  x: distance to nearest point in range [0, sqrt(2)]
 *          y: hash of the point in range [0, 1]
 */
inline vec2 worleyVoronoiNoise(vec2 pos, float seed) {
    vec2 centerCellPos = floor(pos);
    vec2 inCellOffset = fract(pos);
    float hashed;
    float dist = 2.0f;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            float d = length(hash23(seededCellPos) + vec2(x, y) - inCellOffset);
            if (d < dist) {
                dist = d;
                hashed = hash13(seededCellPos);
            }
        }
    }
    return vec2(dist, hashed);
	
}

#endif // !RW_WORLEY_VORONOI_NOISE_GLSL