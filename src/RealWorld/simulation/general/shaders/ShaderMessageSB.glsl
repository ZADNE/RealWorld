/*!
 *  @author     Dubsky Tomas
 */
#ifndef SHADER_MESSAGE_SB_GLSL
#define SHADER_MESSAGE_SB_GLSL

const int k_messageBufferSize = 511;
layout (set = 0, binding = k_shaderMessageBinding, std430)
restrict buffer ShaderMessageSB {
    int totalInts;
    int messages[k_messageBufferSize];
} b_message;

const int k_messageIdRemoveFromSelSlot = 1;
const int k_messageIdDropBlocks        = 2;
const int k_messageIdDropWalls         = 3;

void messageRemoveFromSelSlot(int count){
    int writeBase = atomicAdd(b_message.totalInts, 2);
    if (writeBase + 2 <= k_messageBufferSize) {
        b_message.messages[writeBase] = k_messageIdRemoveFromSelSlot;
        b_message.messages[writeBase + 1] = count;
    } else {
        atomicAdd(b_message.totalInts, -2);
    }
}

int messageDropTiles(uint layer, ivec2 basePosTi, int count){
    int writeBase = atomicAdd(b_message.totalInts, 4 + count);
    if (writeBase + 4 + count <= k_messageBufferSize) {
        b_message.messages[writeBase] = k_messageIdDropBlocks + int(layer);
        b_message.messages[writeBase + 1] = count;
        b_message.messages[writeBase + 2] = basePosTi.x;
        b_message.messages[writeBase + 3] = basePosTi.y;
        return writeBase + 4;
    } else {
        atomicAdd(b_message.totalInts, -(4 + count));
        return -1;
    }
}

#endif // !SHADER_MESSAGE_SB_GLSL