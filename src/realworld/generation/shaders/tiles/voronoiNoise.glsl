/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_VORONOI_NOISE_GLSL
#define RW_VORONOI_NOISE_GLSL
#include <RealWorld/generation/external_shaders/float_hash.glsl>

/**
 * @brief   Calculates Voronoi noise as feature position and distance to it
 * @details The grid is unit-sized
 * @return  x, y: position of the nearest feature
 *          z:    distance to the nearest feature in range [0, sqrt(2)]
 */
inline vec3 voronoiFeatureNoise(vec2 pos, float seed) {
    vec2 centerCellPos = floor(pos);
    vec2 inCellOffset = fract(pos);
    vec2 featPos;
    float distSqr = 4.0f;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            vec2 fPos = hash23(seededCellPos) + vec2(x, y);
            vec2 offsetToPos = fPos - inCellOffset;
            float dSqr = dot(offsetToPos, offsetToPos);
            if (dSqr < distSqr) {
                distSqr = dSqr;
                featPos = centerCellPos + fPos;
            }
        }
    }
    return vec3(featPos, sqrt(distSqr));
}

/**
 * @brief   Calculates Voronoi noise as feature position and distance to border
 * @details The grid is unit-sized
 * @return  x, y: position of the nearest feature
 *          z: distance to the nearest border in range [0, sqrt(2)]
 */
inline vec3 voronoiBorderNoise(vec2 pos, float seed) {
    vec2 centerCellPos = floor(pos);
    vec2 inCellOffset = fract(pos);
    vec2 featPos;
    float dist = 4.0f;
    vec2 nearestFeature;

    // Distance to features
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 cellPos = centerCellPos + vec2(x, y);
            vec3 seededCellPos = vec3(cellPos, seed);
            vec2 fPos = hash23(seededCellPos) + vec2(x, y);
            vec2 toFeature = fPos - inCellOffset;
            float dSqr = dot(toFeature, toFeature);
            if (dSqr < dist) {
                dist = dSqr;
                nearestFeature = toFeature;
                featPos = centerCellPos + fPos;
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

    return vec3(featPos, sqrt(dist));
}

#endif // !RW_VORONOI_NOISE_GLSL