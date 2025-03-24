/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_RANDOM_GLSL
#define RW_RANDOM_GLSL

uint g_randomState;

// Xorshift algorithm by George Marsaglia
uint random(){
    g_randomState ^= g_randomState << 13;
    g_randomState ^= g_randomState >> 17;
    g_randomState ^= g_randomState << 5;
    return g_randomState;
}

uvec2 random2(){
    return uvec2(random(), random());
}

bool randomBool(){
    return bool(random() & 1);
}

float randomFloat(){
    const float k_scaler = 1.0 / 0xffffff;
    return float(random() & 0xffffff) * k_scaler;
}

bool probTest(float probability){ return randomFloat() < probability; }
bool probTest1023(uint probability){ return (random() & 1023) < probability; }
bool probTest65535(uint probability){ return (random() & 65535) < probability; }

#endif // !RW_RANDOM_GLSL