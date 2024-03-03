/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_TEMPLATES_UB_GLSL
#define VEG_TEMPLATES_UB_GLSL
#include <RealWorld/constants/generation.glsl>

struct String {
    uint begin;
    uint count;
};

struct ProbabilisticRewriteRuleBodies {
    uint firstIndex;
    uint bodyCount;
};

const int k_rewriteableSymbolCount = 1;

struct VegTemplate {
    String  axiom;
    vec2    initSizeTi;
    vec2    sizeChange;
    vec2    densityStiffness;
    uint    wallType;
    uint    iterCount;
    ProbabilisticRewriteRuleBodies rules[k_rewriteableSymbolCount];
};

#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
const uint8_t   k_sBranch       = uint8_t(0);
const uint8_t   k_sStem         = uint8_t(1);
const uint8_t   k_sLeftTurn     = uint8_t(2);
const uint8_t   k_sRightTurn    = uint8_t(3);
const uint8_t   k_sPush         = uint8_t(4);
const uint8_t   k_sPop          = uint8_t(5);

const int k_totalRewriteRuleBodyCount = 3;

#extension GL_EXT_scalar_block_layout : require
layout (set = 0, binding = k_vegTemplatesBinding, std430)
restrict uniform VegTemplatesUB {
    uint8_t     symbols[k_vegTemplatesSymbolCount];
    float       symbolParams[k_vegTemplatesSymbolCount];
    float       ruleProbs[k_totalRewriteRuleBodyCount];
    String      ruleBodies[k_totalRewriteRuleBodyCount];
    VegTemplate tmplts[k_vegTemplateCount];
} u_vegTmplts;

#endif // !VEG_TEMPLATES_UB_GLSL