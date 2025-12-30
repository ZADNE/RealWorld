/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_GLSL
#define RW_SKY_GLSL

const float k_sunriseSd = 6.0f / 24.0f;
const float k_fullSunlightSd = 10.5f / 24.0f;

/// Light that is always present
const vec3 k_backgroundLight = vec3(0.008f, 0.020f, 0.073f) / 1024.0f;

const vec3 k_sunsetSkyColor = vec3(1.0f, 0.624f, 0.063f);

// Converts time in days to sun-symetric days
// Linearly ascends from midnight (= 0) to the noon (= 0.5),
// then linearly descends to the next midnight (= 0)
//  0.5>        ^           ^       
//            /   \       /   \     
//          /       \   /       \   
//    0>  v           v           v 
//        ^0    ^0.5  ^1    ^1.5  ^2
float dToSd(float tD) {
    float tDf = fract(tD);
    return tDf > 0.5f ? 1.0f - tDf : tDf;
}

// Converts hours to days
float hToD(float tH) {
    return tH * 0.04166666666f;
}

#endif // !RW_SKY_GLSL