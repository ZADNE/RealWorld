/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_VORONOI_NOISE_GLSL
#define RW_VORONOI_NOISE_GLSL
#include <RealWorld/generation/external_shaders/float_hash.glsl>

/**
 * @brief   Calculates Voronoi noise as distance to feature and its hash
 * @details The grid is unit-sized
 * @return  x: distance to nearest feature in range [0, sqrt(2)]
 *          y: hash of the feature in range [0, 1]
 */
inline vec2 voronoiFeatureNoise(vec2 pos, float seed) {
    vec2 centerCellPos = floor(pos);
    vec2 inCellOffset = fract(pos);
    float hashed;
    float distSqr = 2.0f;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            vec2 offsetToPos = hash23(seededCellPos) + vec2(x, y) - inCellOffset;
            float dSqr = dot(offsetToPos, offsetToPos);
            if (dSqr < distSqr) {
                distSqr = dSqr;
                hashed = hash13(seededCellPos);
            }
        }
    }
    return vec2(sqrt(distSqr), hashed);
}

/**
 * @brief   Calculates Voronoi noise as distance to border and feature hash
 * @details The grid is unit-sized
 * @return  x: distance to the nearest border in range [0, sqrt(2)]
 *          y: hash of the nearest border in range [0, 1]
 */
inline vec2 voronoiBorderNoise(vec2 pos, float seed) {
    vec2 centerCellPos = floor(pos);
    vec2 inCellOffset = fract(pos);
    float hashed;
    float dist = 4.0f;
    vec2 nearestFeature;

    // Distance to features
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            vec2 toFeature = hash23(seededCellPos) + vec2(x, y) - inCellOffset;
            float dSqr = dot(toFeature, toFeature);
            if (dSqr < dist) {
                dist = dSqr;
                hashed = hash13(seededCellPos);
                nearestFeature = toFeature;
            }
        }
    }

    // Distance to borders
    dist = 4.0f;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            vec2 toFeature = hash23(seededCellPos) + vec2(x, y) - inCellOffset;
            vec2 toFeatFromNearest = toFeature - nearestFeature;
            if (dot(toFeatFromNearest, toFeatFromNearest) > 0.00001f) {// Skip the same cell
                dist = min(
                    dot(0.5f * (nearestFeature + toFeature), normalize(toFeatFromNearest)),
                    dist
                );
            }
        }
    }

    return vec2(sqrt(dist), hashed);
}

#endif // !RW_VORONOI_NOISE_GLSL