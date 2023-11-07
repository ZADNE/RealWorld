/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2  o_posTi;
layout (location = 1) out vec2  o_sizeTi;
layout (location = 2) out float o_startAngleNorm;
layout (location = 3) out float o_endAngleNorm;
layout (location = 4) out uint  o_branchIndex0wallType16;

const int BranchVectorSBWrite_BINDING = 0;
const int BranchVectorSBRead_BINDING = 1;
#include <RealWorld/vegetation/shaders/BranchVectorSB.glsl>

void main(){
    // Outputs for next stage
    Branch b = b_branchesRead[gl_VertexIndex];
    o_posTi = b.absPosTi;
    o_sizeTi = vec2(b.radiusTi * 2.0, b.lengthTi);
    o_endAngleNorm = b.absAngleNorm;
    o_startAngleNorm = b_branchesRead[gl_VertexIndex + b.parentIndexOffset].absAngleNorm;
    o_branchIndex0wallType16 = gl_VertexIndex | (b.wallType << 16);
}
