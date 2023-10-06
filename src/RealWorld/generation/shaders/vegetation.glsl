/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEGETATION_GLSL
#define VEGETATION_GLSL

struct VegTemplate{
    uint branchFirstIndex;
    uint branchCount;
    float rndLength;
    float rndAngle;
};

const VegTemplate k_vegTemplates[] = {
    VegTemplate(0,  51, 0.325,  0.03125),   // Oak
    VegTemplate(51, 64, 0.75,   0.0625),    // Acacia
    VegTemplate(115, 64, 0.75,  0.03125)    // Tall grass
};

const uint k_vegTemplatesBranchCount =
    k_vegTemplates[k_vegTemplates.length() - 1].branchFirstIndex + 
    k_vegTemplates[k_vegTemplates.length() - 1].branchCount;

// Describes vegetation distribution
struct Vegetation {
    // Represents probabilities to generate each template
    // Probabilities are per chunk
    float genProbability[k_vegTemplates.length()];
};

const Vegetation k_mountainVeg =    {{  0.01,   0.0,    0.0}};
const Vegetation k_tundraVeg =      {{  0.01,   0.0,    0.0}};
const Vegetation k_taigaVeg =       {{  0.01,   0.0,    0.0}};

const Vegetation k_grasslandVeg =   {{  0.1,    0.0,    1.0}};
const Vegetation k_forestVeg =      {{  2.0,    0.0,    1.0}};
const Vegetation k_swampVeg =       {{  0.0,    0.0,    0.0}};

const Vegetation k_desertVeg =      {{  0.0,    0.1,    0.0}};
const Vegetation k_savannaVeg =     {{  0.0,    1.0,    1.0}};
const Vegetation k_rainforestVeg =  {{  0.0,    1.0,    1.0}};

const Vegetation k_biomeVegetations[3][3] = {
//humidity> |low                |normal             |high           temperature \/
    {       k_mountainVeg,      k_tundraVeg,        k_taigaVeg      },  //-low
    {       k_grasslandVeg,     k_forestVeg,        k_swampVeg      },  //-normal
    {       k_desertVeg,        k_savannaVeg,       k_rainforestVeg }   //-high
};


/**
 * @param biomeClimate x = temperature, y = humidity
 */
Vegetation biomeVegetation(vec2 biomeClimate){
    // Calculate coords
    biomeClimate *= vec2(k_biomeVegetations.length() - 1, k_biomeVegetations[0].length() - 1);
    ivec2 ll = ivec2(biomeClimate);
    vec2 frac = fract(biomeClimate);

    // Gather
    Vegetation b00 = k_biomeVegetations[ll.x][ll.y];
    const Vegetation b01 = k_biomeVegetations[ll.x][ll.y + 1];
    const Vegetation b10 = k_biomeVegetations[ll.x + 1][ll.y];
    const Vegetation b11 = k_biomeVegetations[ll.x + 1][ll.y + 1];

    for (int i = 0; i < k_vegTemplates.length(); ++i){
        // Interpolate over X axis
        float bx0 = mix(b00.genProbability[i], b10.genProbability[i], frac.x);
        float bx1 = mix(b01.genProbability[i], b11.genProbability[i], frac.x);

        // Interpolate over Y axis
        b00.genProbability[i] = mix(bx0, bx1, frac.y);
    }

    return b00;
}

#endif // !VEGETATION_GLSL