/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_TEMPLATES_UB_GLSL
#define VEG_TEMPLATES_UB_GLSL
#include <RealWorld/constants/generation.glsl>

struct String {
    uint sBegin; // Index of first symbol
    uint sCount; // Number of symbols
    uint pBegin; // Index of first parameter
    uint pCount; // Number of parameters
};

struct Condition {
    uint  paramIndex;
    float border;
};

struct RuleBodies {
    Condition cond;
    uint firstIndex[2]; // 0 <=> cond false, 1 <=> cond true
    uint bodyCount[2];  // 0 <=> cond false, 1 <=> cond true
};

const int k_rewriteableSymbolCount = 3; // So far: == number of ~segment symbols

struct VegTemplate {
    String     axiom;
    vec2       densityStiffness;
    uint       wallType;
    uint       iterCount;
    RuleBodies rules[k_rewriteableSymbolCount];
};

#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
const uint8_t   k_sTwig         = uint8_t(0);
const uint8_t   k_sBranch       = uint8_t(1);
const uint8_t   k_sStem         = uint8_t(2);
const uint8_t   k_sRotate       = uint8_t(3);
const uint8_t   k_sFlip         = uint8_t(4);
const uint8_t   k_sPush         = uint8_t(5);
const uint8_t   k_sPop          = uint8_t(6);

const uint k_paramCount[7] = {
    2,
    2,
    2,
    1,
    0,
    0,
    0
};

const int k_totalRewriteRuleBodyCount = 8;

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