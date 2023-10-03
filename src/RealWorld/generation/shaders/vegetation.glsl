/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEGETATION_GLSL
#define VEGETATION_GLSL

// Describes vegetation distribution
struct Vegetation {
    // x, y, z = relative distribution of grass, shrubs and trees (sums to 1.0)
    // w       = chance to spawn one of the above (0.0 to 1.0)
    vec4 distr;
};

const Vegetation k_mountainVeg =    {{0.8, 0.2, 0.0, 0.0050}};
const Vegetation k_tundraVeg =      {{0.6, 0.4, 0.0, 0.0200}};
const Vegetation k_taigaVeg =       {{0.2, 0.2, 0.6, 0.0500}};

const Vegetation k_grasslandVeg =   {{0.8, 0.2, 0.0, 0.0250}};
const Vegetation k_forestVeg =      {{0.6, 0.4, 0.0, 0.0750}};
const Vegetation k_swampVeg =       {{0.2, 0.2, 0.6, 0.0500}};

const Vegetation k_desertVeg =      {{0.8, 0.2, 0.0, 0.0005}};
const Vegetation k_savannaVeg =     {{0.6, 0.4, 0.0, 0.0025}};
const Vegetation k_rainforestVeg =  {{0.2, 0.2, 0.6, 0.1000}};

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
    biomeClimate = fract(biomeClimate);
    biomeClimate *= vec2(k_biomeVegetations.length() - 1, k_biomeVegetations[0].length() - 1);
    ivec2 ll = ivec2(biomeClimate);
    vec2 frac = fract(biomeClimate);

    // Gather
    Vegetation b00 = k_biomeVegetations[ll.x][ll.y];
    Vegetation b01 = k_biomeVegetations[ll.x][ll.y + 1];
    Vegetation b10 = k_biomeVegetations[ll.x + 1][ll.y];
    Vegetation b11 = k_biomeVegetations[ll.x + 1][ll.y + 1];

    // Interpolate over X axis
    b00.distr = mix(b00.distr, b10.distr, frac.x);
    b01.distr = mix(b01.distr, b11.distr, frac.x);

    // Interpolate over Y axis
    b00.distr = mix(b00.distr, b01.distr, frac.y);
    return b00;
}




// x = index of first branch, y = branch count
const uvec2 k_vegTemplates[] = {{0, 51}};

const uint k_vegTemplatesBranchCount = 51;

#endif // !VEGETATION_GLSL