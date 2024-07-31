/*!
 *  @author     Dubsky Tomas
 */
#ifndef SHADER_MESSAGE_SB_GLSL
#define SHADER_MESSAGE_SB_GLSL

const int k_messageBufferSize = 63;
layout (set = 0, binding = k_shaderMessageBinding, std430)
restrict buffer ShaderMessageSB {
    int totalInts;
    int messages[k_messageBufferSize];
} b_message;

const int k_messageIdRemoveFromSelSlot = 1;

void messageRemoveFromSelSlot(int count){
    int writeBase = atomicAdd(b_message.totalInts, 2);
    if (writeBase + 2 <= k_messageBufferSize) {
        b_message.messages[writeBase] = k_messageIdRemoveFromSelSlot;
        b_message.messages[writeBase + 1] = count;
    }
}

#endif // !SHADER_MESSAGE_SB_GLSL