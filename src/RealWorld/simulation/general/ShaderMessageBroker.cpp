/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/general/ShaderMessageBroker.hpp>

namespace rw {

enum class ShaderMessageId {
    Reserved,
    RemoveFromSelSlot,
    DropBlocks,
    DropWalls
};

ShaderMessageBroker::ShaderMessageBroker(
    ActionCmdBuf& acb, Inventory& playerInv, ItemUser& itemUser
)
    : m_playerInv(playerInv)
    , m_itemUser(itemUser) {
    m_messageBufMapped->read().totalInts  = 0;
    m_messageBufMapped->write().totalInts = 0;
    acb.track(BufferTrackName::ShaderMessage, m_messageBuf);
}

void ShaderMessageBroker::beginStep(ActionCmdBuf& acb) {
    // Process received messages
    int intCount = m_messageBufMapped->read().totalInts;
    int* ints    = m_messageBufMapped->read().messages;
    for (int i = 0; i < intCount;) {
        auto msgId = static_cast<ShaderMessageId>(ints[i++]);
        switch (msgId) {
        case ShaderMessageId::RemoveFromSelSlot:
            m_itemUser.finishSpecRemoval(ints[i++]);
            break;
        case ShaderMessageId::DropBlocks:
        case ShaderMessageId::DropWalls:
            int section = static_cast<int>(
                msgId == ShaderMessageId::DropBlocks ? ItemIdSection::Blocks
                                                     : ItemIdSection::Walls
            );
            int count   = ints[i++];
            int baseXTi = ints[i++];
            int baseYTi = ints[i++];
            while (count--) {
                int bits = ints[i++];
                glm::ivec2 posTi{
                    baseXTi + (bits >> 24) & 0xff, baseYTi + (bits >> 16) & 0xff
                };
                int tileType = (bits >> 8) & 0xff;
                Item item{static_cast<ItemId>(section | tileType), 1};
                m_playerInv.fill(item, 1.0f, glm::ivec2{}, false);
            }
            m_playerInv.wasChanged();
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
