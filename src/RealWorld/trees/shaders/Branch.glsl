/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_GLSL
#define BRANCH_GLSL

/**
* @see rw::Branch
*/
struct Branch {
    vec2    absPosTi;
    uint    parentIndex;
    //uint    angles;
    float   radiusTi;
    float   lengthTi;
    float   density;
    float   stiffness;
    float   padding;
    vec4    angles;
};

#endif // BRANCH_GLSL
