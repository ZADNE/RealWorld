/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace rw {

#pragma warning(push)
#pragma warning(disable : 4200)
struct ActiveChunksSB {
    glm::ivec2 activeChunksMask;
    glm::ivec2 worldTexSizeCh;
    glm::ivec4 dynamicsGroupSize;
    glm::ivec2 offsets[]; // First indices: offsets of update chunks, in tiles
    // Following indices: absolute positions of chunks, in chunks
};
#pragma warning(pop)

} // namespace rw
