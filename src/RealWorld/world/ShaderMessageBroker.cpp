/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ShaderMessageBroker.hpp>

namespace rw {

enum class ShaderMessageId {
    Reserved,
    RemoveFromSelSlot
};

rw::ShaderMessageBroker::ShaderMessageBroker(ActionCmdBuf& acb, ItemUser& itemUser)
    : m_itemUser(itemUser) {
    m_messageBufMapped->read().totalInts  = 0;
    m_messageBufMapped->write().totalInts = 0;
    acb.track(BufferTrackName::ShaderMessage, m_messageBuf);
}

void ShaderMessageBroker::beginStep(ActionCmdBuf& acb) {
    // Process received messages
    int intCount = m_messageBufMapped->read().totalInts;
    int* ints    = m_messageBufMapped->read().messages;
    for (int i = 0; i < intCount;) {
        switch (static_cast<ShaderMessageId>(ints[i++])) {
        case ShaderMessageId::RemoveFromSelSlot:
            m_itemUser.finishSpecRemoval(ints[i++]);
            break;
        }
    }

    // Clear message buffer buffer
    acb.action(
        [&](const re::CommandBuffer& cb) {
            cb->fillBuffer(*m_messageBuf, 0ull, sizeof(ShaderMessageSB), 0);
        },
        BufferAccess{
            .name   = BufferTrackName::ShaderMessage,
            .stage  = vk::PipelineStageFlagBits2::eTransfer,
            .access = vk::AccessFlagBits2::eTransferWrite
        }
    );
}

void ShaderMessageBroker::endStep(ActionCmdBuf& acb) {
    acb.action( // Copy messages back
        [&](const re::CommandBuffer& cb) {
            vk::BufferCopy2 bufferCopy{
                0ull,
                sizeof(ShaderMessageSB) * re::StepDoubleBufferingState::readIndex(),
                sizeof(ShaderMessageSB)
            };
            cb->copyBuffer2(vk::CopyBufferInfo2{
                m_messageBuf.buffer(),       // Src buffer
                m_messageBufMapped.buffer(), // Dst buffer
                bufferCopy                   // Region
            });
        },
        BufferAccess{
            .name   = BufferTrackName::ShaderMessage,
            .stage  = vk::PipelineStageFlagBits2::eTransfer,
            .access = vk::AccessFlagBits2::eTransferRead
        }
    );
}

} // namespace rw
