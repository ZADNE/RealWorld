/*!
 *  @author     Dubsky Tomas
 */

struct Root {
    vec2 posPx;
};

layout(set = 0, binding = RootsSB_BINDING, std430) restrict buffer RootsSB {
    Root b_roots[];
};
