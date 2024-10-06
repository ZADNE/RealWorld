/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

namespace rw {

/**
 * @brief Moves the view at constant speed to test performance
 */
glm::vec2 WorldRoom::newViewPos() const {
    static glm::vec2 pos{0.0f};
    pos += glm::vec2{15.0f, 0.0f}; // NOLINT(*-magic-numbers)
    return pos;
}

} // namespace rw
