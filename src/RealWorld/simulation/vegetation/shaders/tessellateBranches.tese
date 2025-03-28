/**
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec2 o_tipDir;
layout (location = 2) out vec2 o_sizeTi;
layout (location = 3) out uint o_branchIndex15wallType31;

layout (quads, fractional_odd_spacing) in;
layout (location = 0) patch in vec2  i_p0Ti; // Start
layout (location = 1) patch in vec2  i_p1Ti; // Control
layout (location = 2) patch in vec2  i_p2Ti; // End
layout (location = 3) patch in vec2  i_sizeTi;
layout (location = 4) patch in uint  i_branchIndex15wallType31;

#include <RealWorld/simulation/vegetation/shaders/VegDynamicsPC.glsl>

const float k_1Over2Pi = 0.15915494309;

void main(){
    // Helper vars
    float t = gl_TessCoord.y;
    float ti = 1.0 - t;
    vec2 p2_1 = i_p2Ti - i_p1Ti;

    // Position in axis of the branch
    vec2 pTi = i_p1Ti + ti * ti * (i_p0Ti - i_p1Ti) + t * t * p2_1;

    // Diameter offset
    o_tipDir = normalize(ti * (i_p1Ti - i_p0Ti) + t * p2_1);
    vec2 normal = vec2(o_tipDir.y, -o_tipDir.x);
    vec2 diaOffsetTi = normal * i_sizeTi.x * (gl_TessCoord.x - 0.5);

    // Final position
    gl_Position = p_.mvpMat * vec4(pTi + diaOffsetTi, 0.0, 1.0);

    // Other attributes
    o_uv = gl_TessCoord.xy;
    o_sizeTi = i_sizeTi;
    o_branchIndex15wallType31 = i_branchIndex15wallType31;
}
