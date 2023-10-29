/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2 o_uv;
layout (location = 1) out uint o_branchIndex0parentDiscr16wallType24;

layout (quads, fractional_even_spacing) in;
layout (location = 0) patch in vec2  i_p0Ti; // Start
layout (location = 1) patch in vec2  i_p1Ti; // Control
layout (location = 2) patch in vec2  i_p2Ti; // End
layout (location = 3) patch in vec2  i_sizeTi;
layout (location = 4) patch in uint  i_branchIndex0parentDiscr16wallType24;

#include <RealWorld/vegetation/shaders/VegDynamicsPC.glsl>

void main(){
    // Helper vars
    float t = gl_TessCoord.y;
    float ti = 1.0 - t;
    vec2 p2_1 = i_p2Ti - i_p1Ti;

    // Position in axis of the branch
    vec2 pTi = i_p1Ti + ti * ti * (i_p0Ti - i_p1Ti) + t * t * p2_1;

    // Diameter offset
    vec2 tTi = ti * (i_p1Ti - i_p0Ti) + t * p2_1;
    vec2 normal = normalize(vec2(tTi.y, -tTi.x));
    vec2 diaOffsetTi = normal * i_sizeTi.x * (gl_TessCoord.x - 0.5);

    // Final position
    gl_Position = p_mvpMat * vec4(pTi + diaOffsetTi, 0.0, 1.0);

    // Other attributes
    o_uv = gl_TessCoord.xy * (i_sizeTi - 1.0);
    o_branchIndex0parentDiscr16wallType24 = i_branchIndex0parentDiscr16wallType24;
}
