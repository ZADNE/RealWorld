/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(location = 0) out        vec2 o_botLeftPx;
layout(location = 1) out        vec2 o_velPx;
layout(location = 2) out flat   uint o_layerTypeVar;

layout(location = 0) in         vec2 i_botLeftPx;
layout(location = 1) in         vec2 i_velPx;
layout(location = 2) in         uint i_layerTypeVar;

void main() {
    o_botLeftPx    = i_botLeftPx;
    o_velPx        = i_velPx;
    o_layerTypeVar = i_layerTypeVar;
}
