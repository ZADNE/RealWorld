/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEG_DISTRIBUTION_GLSL
#define VEG_DISTRIBUTION_GLSL
#include <RealWorld/constants/generation.glsl>

// Describes vegetation distribution
struct VegDistr {
    // Represents probabilities to generate each template
    // Probabilities are per chunk
    float genProbability[k_vegTemplateCount];
};

const VegDistr k_mountainVeg =    {{  0.0,    0.0,    0.0}};
const VegDistr k_tundraVeg =      {{  0.0,    0.0,    0.0}};
const VegDistr k_taigaVeg =       {{  0.0,    0.0,    0.0}};

const VegDistr k_grasslandVeg =   {{  0.1,    0.1,    0.0}};
const VegDistr k_forestVeg =      {{  1.0,    0.0,    0.0}};
const VegDistr k_swampVeg =       {{  0.0,    0.0,    0.0}};

const VegDistr k_desertVeg =      {{  0.0,    0.1,    0.0}};
const VegDistr k_savannaVeg =     {{  0.0,    0.7,    0.0}};
const VegDistr k_rainforestVeg =  {{  0.0,    0.0,    0.0}};

const VegDistr k_biomeVegDistrs[3][3] = {
//humidity> |low                |normal             |high           temperature \/
    {       k_mountainVeg,      k_tundraVeg,        k_taigaVeg      },  //-low
    {       k_grasslandVeg,     k_forestVeg,        k_swampVeg      },  //-normal
    {       k_desertVeg,        k_savannaVeg,       k_rainforestVeg }   //-high
};


/**
 * @param biomeClimate x = temperature, y = humidity
 */
VegDistr biomeVegDistr(vec2 biomeClimate){
    // Calculate coords
    biomeClimate *= vec2(k_biomeVegDistrs.length() - 1, k_biomeVegDistrs[0].length() - 1);
    ivec2 ll = ivec2(biomeClimate);
    vec2 frac = fract(biomeClimate);

    // Gather
    VegDistr b00 = k_biomeVegDistrs[ll.x][ll.y];
    const VegDistr b01 = k_biomeVegDistrs[ll.x][ll.y + 1];
    const VegDistr b10 = k_biomeVegDistrs[ll.x + 1][ll.y];
    const VegDistr b11 = k_biomeVegDistrs[ll.x + 1][ll.y + 1];

    for (int i = 0; i < k_vegTemplateCount; ++i){
        // Interpolate over X axis
        float bx0 = mix(b00.genProbability[i], b10.genProbability[i], frac.x);
        float bx1 = mix(b01.genProbability[i], b11.genProbability[i], frac.x);

        // Interpolate over Y axis
        b00.genProbability[i] = mix(bx0, bx1, frac.y);
    }

    return b00;
}

#endif // !VEG_DISTRIBUTION_GLSL