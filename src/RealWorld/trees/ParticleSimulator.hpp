/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class ParticleSimulator {
public:
    void adoptSave(const MetadataSave& save, const glm::ivec2& worldTexSizeCh);

private:
    // std::optional
};

} // namespace rw