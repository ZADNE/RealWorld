/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2  o_posTi;
layout (location = 1) out vec2  o_sizeTi;
layout (location = 2) out float o_startAngleNorm;
layout (location = 3) out float o_endAngleNorm;
layout (location = 4) out uint  o_branchIndex15wallType31;

#include <RealWorld/vegetation/shaders/wind.glsl>
#include <RealWorld/vegetation/shaders/branchRasterizationPll.glsl>
#include <RealWorld/vegetation/shaders/BranchSB.glsl>
#include <RealWorld/vegetation/shaders/normAngles.glsl>
#include <RealWorld/vegetation/shaders/VegDynamicsPC.glsl>

#include <RealWorld/generation/external_shaders/snoise.glsl>
const float k_third = 0.33333333333;

void main(){
    // Load this branch
    const uint id = gl_VertexIndex;
    const uvec2 parentOffsetWallType = loadBranchParentOffsetWallType(id);
    const float radiusTi = b_branch.radiusTi[id];
    const float lengthTi = b_branch.lengthTi[id];
    float absAngNorm = b_branch.absAngNorm[p_readBuf][id];
    vec2  posTi = b_branch.absPosTi[p_readBuf][id];
    const vec2 densityStiffness = b_branch.densityStiffness[id];
    float  angVel = b_branch.angVel[id];

    // Load parent
    const uint pId = id - int(parentOffsetWallType.x);
    float pAbsAngNorm = b_branch.absAngNorm[p_readBuf][pId];

    // Outputs for next stage
    o_posTi = posTi;
    o_sizeTi = vec2(radiusTi * 2.0, lengthTi);
    o_endAngleNorm = absAngNorm;
    o_startAngleNorm = pAbsAngNorm;
    o_branchIndex15wallType31 = id | (parentOffsetWallType.y << 16);

    // Simulation
    float wind = windStrength(posTi, p_timeSec);

    float volume = k_pi * radiusTi * radiusTi * lengthTi;
    float weight = volume * densityStiffness.x;

    vec2 force = vec2(wind * radiusTi * lengthTi * 0.1, weight * -0.01);

    float forceAngle = atan(force.y, force.x);
    float forceSize  = length(force);

    float momentOfInertia = k_third * weight * lengthTi *
                            lengthTi;
    momentOfInertia = momentOfInertia == 0.0 ? 1.0 : momentOfInertia;

    float angularAcc =
        lengthTi * forceSize *
        sin(forceAngle - absAngNorm * k_2pi) /
        momentOfInertia;

    float angleDiffToRestNorm = angularDifference(
        fract(pAbsAngNorm + b_branch.relRestAngNorm[id]), absAngNorm
    );

    angularAcc += densityStiffness.y * angleDiffToRestNorm - angVel * 0.875;

    // Rotate branch (if not root)
    if (parentOffsetWallType.x != 0){
        angVel += angularAcc;
        absAngNorm += angVel;
        absAngNorm = fract(absAngNorm);
    }

    // Shift branch
    const vec2  pPosTi = b_branch.absPosTi[p_readBuf][pId];
    const float pLengthTi = b_branch.lengthTi[pId];
    posTi = pPosTi + toCartesian(pLengthTi, pAbsAngNorm);

    // Store the modified branch
    const uint writeBuf = 1 - p_readBuf;
    b_branch.absPosTi[writeBuf][id] = posTi;
    b_branch.absAngNorm[writeBuf][id] = absAngNorm;
    b_branch.angVel[id] = angVel;
}
