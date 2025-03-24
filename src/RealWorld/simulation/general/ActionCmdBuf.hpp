/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/commands/ActionCommandBuffer.hpp>

namespace rw {

enum class BufferTrackName {
    ActiveChunks,
    Branch,
    AllocReg,
    ShaderMessage
};
using BufferAccess = re::BufferAccess<BufferTrackName>;

enum class ImageTrackName {
    World
};
using ImageAccess = re::ImageAccess<ImageTrackName>;

/**
 * @brief Is action command buffer of RealWorld
 */
using ActionCmdBuf = re::ActionCommandBuffer<BufferTrackName, 4, ImageTrackName, 1>;

} // namespace rw
