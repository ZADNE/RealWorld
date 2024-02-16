/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEG_DISTRIBUTION_GLSL
#define VEG_DISTRIBUTION_GLSL

struct VegTemplate{
    uint branchFirstIndex;
    uint branchCount;
    float rndLength;
    float rndAngle;
};

const VegTemplate k_vegTemplates[] = {
    VegTemplate(0,      51, 0.325,  0.03125),   // Oak
    VegTemplate(51,     64, 0.75,   0.0625),    // Acacia
    VegTemplate(115,    64, 0.75,   0.03125),   // Wheat (must be index 2; see generateVeg)
};

const uint k_vegTemplatesBranchCount =
    k_vegTemplates[k_vegTemplates.length() - 1].branchFirstIndex + 
    k_vegTemplates[k_vegTemplates.length() - 1].branchCount;

// Describes vegetation distribution
struct VegDistr {
    // Represents probabilities to generate each template
    // Probabilities are per chunk
    float genProbability[k_vegTemplates.length()];
};

const VegDistr k_mountainVeg =    {{  0.01,   0.0,    0.0}};
const VegDistr k_tundraVeg =      {{  0.01,   0.0,    0.0}};
const VegDistr k_taigaVeg =       {{  0.01,   0.0,    0.0}};

const VegDistr k_grasslandVeg =   {{  0.1,    0.0,    1.0}};
const VegDistr k_forestVeg =      {{  2.0,    0.0,    1.0}};
const VegDistr k_swampVeg =       {{  0.0,    0.0,    0.0}};

const VegDistr k_desertVeg =      {{  0.0,    0.1,    0.0}};
const VegDistr k_savannaVeg =     {{  0.0,    1.0,    1.0}};
const VegDistr k_rainforestVeg =  {{  0.0,    1.0,    1.0}};

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

    for (int i = 0; i < k_vegTemplates.length(); ++i){
        // Interpolate over X axis
        float bx0 = mix(b00.genProbability[i], b10.genProbability[i], frac.x);
        float bx1 = mix(b01.genProbability[i], b11.genProbability[i], frac.x);

        // Interpolate over Y axis
        b00.genProbability[i] = mix(bx0, bx1, frac.y);
    }

    return b00;
}

#endif // !VEG_DISTRIBUTION_GLSL