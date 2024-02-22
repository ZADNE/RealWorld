/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/commands/ActionCommandBuffer.hpp>

namespace rw {

enum class BufferTrackName { ActiveChunks, Branch, AllocReg };
using BufferAccess = re::BufferAccess<BufferTrackName>;

enum class ImageTrackName { World };
using ImageAccess = re::ImageAccess<ImageTrackName>;

using ActionCmdBuf = re::ActionCommandBuffer<BufferTrackName, 3, ImageTrackName, 1>;

} // namespace rw