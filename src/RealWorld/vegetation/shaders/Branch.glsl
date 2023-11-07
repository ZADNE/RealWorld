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
    int     parentIndexOffset;
    uint    wallType;
    float   absAngleNorm;
    float   relRestAngleNorm;
    float   angleVelNorm;
    float   radiusTi;
    float   lengthTi;
    float   density;
    float   stiffness;
    uint    padding;
};

#endif // !BRANCH_GLSL
