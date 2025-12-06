/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_GLSL
#define RW_SKY_GLSL

const vec3 k_descentStart = vec3(13.5f, 13.5f, 13.5f) / 24.0f;
const vec3 k_fromNoonToDescent = k_descentStart - 0.5f;
const vec3 k_sunset = vec3(18.0f, 18.0f, 18.0f) / 24.0f;
const vec3 k_fromNoonToSunset = k_sunset - 0.5f;

#endif // !RW_SKY_GLSL