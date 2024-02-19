/*!
 *  @author    Dubsky Tomas
 */
#version 460
layout (triangle_strip, max_vertices = 12) out;
layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec2 o_normal;
layout (location = 2) out uint o_branchIndex15wallType31;

layout (triangles) in;
layout (location = 0) in vec2 i_uv[];
layout (location = 1) in vec2 i_normal[];
layout (location = 2) in uint i_branchIndex15wallType31[];

void duplicationPass(vec2 offset){
    for (int i = 0; i < 3; ++i){
        vec4 pos = gl_in[i].gl_Position;
        pos.xy += offset;
        gl_Position = pos;
        o_uv = i_uv[i];
        o_normal = i_normal[i];
        o_branchIndex15wallType31 = i_branchIndex15wallType31[0];
        EmitVertex();
    }
    EndPrimitive();
}

void main() {
    // The main copy (with analysis)
    vec2 offset = vec2(0.0);
    for (int i = 0; i < 3; ++i){
        const vec4 pos = gl_in[i].gl_Position;
        offset = mix( // Remember if the vertex was outside range
            offset,
            vec2(greaterThan(abs(pos.xy), vec2(1.0))) * sign(pos.xy),
            equal(offset, vec2(0.0))
        );
        gl_Position = pos;
        o_normal = i_normal[i];
        o_branchIndex15wallType31 = i_branchIndex15wallType31[0];
        o_uv = i_uv[i];
        EmitVertex();
    }
    EndPrimitive();
    offset *= -2.0;

    if (offset.x != 0.0){ // Horizontal duplication
        duplicationPass(vec2(offset.x, 0.0));
    }

    if (offset.y != 0.0){ // Vertical duplication
        duplicationPass(vec2(0.0, offset.y));
    }

    if (offset.x != 0.0 && offset.y != 0.0){ // Diagonal duplication
        duplicationPass(vec2(offset.x, offset.y));
    }
}
