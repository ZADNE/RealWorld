/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/general/ShaderMessageBroker.hpp>

namespace rw {

constexpr int k_byteBits = CHAR_BIT;

enum class ShaderMessageID {
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
    int* ints = m_messageBufMapped->read().messages; // NOLINT(*-pointer-decay)
    for (int i = 0; i < intCount;) {
        auto msgID = static_cast<ShaderMessageID>(ints[i++]);
        switch (msgID) {
        case ShaderMessageID::Reserved: break;
        case ShaderMessageID::RemoveFromSelSlot:
            m_itemUser.finishSpecRemoval(ints[i++]);
            break;
        case ShaderMessageID::DropBlocks:
        case ShaderMessageID::DropWalls:
            int section = static_cast<int>(
                msgID == ShaderMessageID::DropBlocks ? ItemIDSection::Blocks
                                                     : ItemIDSection::Walls
            );
            int count   = ints[i++];
            int baseXTi = ints[i++];
            int baseYTi = ints[i++];
            while (count--) {
                int bits = ints[i++];
                glm::ivec2 posTi{
                    baseXTi + (bits >> (k_byteBits * 3)) & 0xff,
                    baseYTi + (bits >> (k_byteBits * 2)) & 0xff
                };
                int tileType = (bits >> k_byteBits) & 0xff;
                Item item{static_cast<ItemID>(section | tileType), 1};
                m_playerInv.fill(item, 1.0f, glm::ivec2{}, false);
            }
            m_playerInv.wasChanged();
            break;
        }
    }

    // Clear message buffer buffer
    acb.action(
        [&](const re::CommandBuffer& cb) {
            cb->fillBuffer(*m_messageBuf, 0ull, sizeof(glsl::ShaderMessageSB), 0);
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
                sizeof(glsl::ShaderMessageSB) *
                    re::StepDoubleBufferingState::readIndex(),
                sizeof(glsl::ShaderMessageSB)
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
