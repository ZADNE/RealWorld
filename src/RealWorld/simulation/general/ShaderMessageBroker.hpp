/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>

#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/simulation/general/ActionCmdBuf.hpp>

namespace rw {

/**
 * @brief Allows shaders to send messages back to C++ systems
 */
class ShaderMessageBroker {
public:

    ShaderMessageBroker(ActionCmdBuf& acb, Inventory& playerInv, ItemUser& itemUser);

    /**
     * @brief Processes messages sent last step
     */
    void beginStep(ActionCmdBuf& acb);

    /**
     * @brief Sends messages to be received next step
     */
    void endStep(ActionCmdBuf& acb);

    const re::Buffer& messageBuffer() const { return m_messageBuf; }

private:
    constexpr static int k_messageBufferSize = 511;
    struct ShaderMessageSB {
        int totalInts{};
        std::array<int, k_messageBufferSize> messages;
    };

    using enum vk::BufferUsageFlagBits;
    re::Buffer m_messageBuf{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(ShaderMessageSB),
        .usage       = eStorageBuffer | eTransferSrc | eTransferDst,
        .debugName   = "rw::ShaderMessageBroker::messageBuf"
    }};

    using enum vma::AllocationCreateFlagBits;
    using MappedType = re::StepDoubleBuffered<ShaderMessageSB>;
    re::BufferMapped<MappedType> m_messageBufMapped{re::BufferCreateInfo{
        .allocFlags  = eMapped | eHostAccessRandom,
        .memoryUsage = vma::MemoryUsage::eAutoPreferHost,
        .sizeInBytes = sizeof(MappedType),
        .usage       = eTransferDst,
        .debugName   = "rw::ShaderMessageBroker::messageBufMapped"
    }};

    Inventory& m_playerInv;
    ItemUser& m_itemUser;
};

} // namespace rw
