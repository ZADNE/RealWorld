#extension GL_EXT_scalar_block_layout : require
#include <RealWorld/reserved_units/buffers.glsl>
layout(std430, binding = UNIF_BUF_WORLDDYNAMICS) restrict uniform WorldDynamicsUIB {
    ivec2 globalPosTi;
    uint modifyTarget;
    uint modifyShape;
    uvec2 modifySetValue;
    float modifyDiameter;
    uint timeHash;
    ivec2 updateOrder[16];
};
