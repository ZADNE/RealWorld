/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/generation/GenerationPC.hpp>

namespace rw {

/**
 * @brief Generates new chunks by compute shaders.
 */
class TreeGenerator {
public:
    TreeGenerator(GenerationPC& genPC);

private:
    GenerationPC& m_genPC;
};

} // namespace rw