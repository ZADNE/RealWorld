/*!
 *  @author     Dubsky Tomas
 */
layout(set = 0, binding = LSystemSB_BINDING, std430) restrict buffer LSystemSB {
    uint b_symbols[1024];
    float b_values[1024];
};
