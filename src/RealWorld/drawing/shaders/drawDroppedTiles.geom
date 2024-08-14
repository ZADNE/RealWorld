/*!
 *  @author    Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/drawing/shaders/DroppedTilesDrawingPC.glsl>

layout (set = 0, binding = 0)
uniform sampler2D u_layerAtlas[2];

layout (triangle_strip, max_vertices = 4) out;
layout (location = 0) out vec4 o_col;

layout (points) in;
layout(location = 0) in vec2 i_botLeftPx[];
layout(location = 1) in vec2 i_velPx[];
layout(location = 2) in uint i_layerTypeVar[];

void main() {
    vec2 botLeftPx = i_botLeftPx[0] + p_interpFactor * i_velPx[0];
    uint layerTypeVar = i_layerTypeVar[0];
    uint layer = layerTypeVar >> 16;
    ivec2 typeVar = ivec2(layerTypeVar & 0xff, (layerTypeVar >> 8) & 0xff);
    vec4 layerCol = texelFetch(u_layerAtlas[layer], typeVar, 0);
    layerCol.rgb += p_blinkState;
    for (int i = 0; i < 4; ++i){
        vec2 pos = botLeftPx + tiToPx(vec2(i & 1, i >> 1));
        gl_Position = p_mvpMat * vec4(pos, 0.0, 1.0);
        o_col = layerCol;
        EmitVertex();
    }
    EndPrimitive();
}
