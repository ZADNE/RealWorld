/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2  o_posTi;
layout (location = 1) out vec2  o_sizeTi;
layout (location = 2) out float o_startAngleNorm;
layout (location = 3) out float o_endAngleNorm;
layout (location = 4) out uint  o_branchIndex15wallType31;

const int BranchSB_BINDING = 0;
#include <RealWorld/vegetation/shaders/BranchSB.glsl>
#include <RealWorld/vegetation/shaders/VegDynamicsPC.glsl>

void main(){
    // Load this branch
    const uint id = gl_VertexIndex;
    const uvec2 parentOffsetWallType = loadBranchParentOffsetWallType(id);
    const float radiusTi = b_branch.radiusTi[id];
    const float lengthTi = b_branch.lengthTi[id];
    const float absAngNorm = b_branch.absAngNorm[p_readBuf][id];

    // Load parent
    const uint pId = id - int(parentOffsetWallType.x);
    const float pAbsAngNorm = b_branch.absAngNorm[p_readBuf][pId];

    // Outputs for next stage
    o_posTi = b_branch.absPosTi[p_readBuf][id];
    o_sizeTi = vec2(radiusTi * 2.0, lengthTi);
    o_endAngleNorm = absAngNorm;
    o_startAngleNorm = pAbsAngNorm;
    o_branchIndex15wallType31 = id | (parentOffsetWallType.y << 16);
}
