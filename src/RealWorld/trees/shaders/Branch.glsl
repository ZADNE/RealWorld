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
    uint    angles;
    float   radiusTi;
    float   lengthTi;
    float   density;
    float   stiffness;
};

#endif // BRANCH_GLSL