/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/WorldRoom.hpp>

namespace rw {

glm::vec2 WorldRoom::newViewPos() const {
    glm::vec2 prevViewPos   = m_worldView.center();
    glm::vec2 targetViewPos = glm::vec2(m_player.center()) * 0.75f +
                              m_worldView.cursorRel() * 0.25f;
    return prevViewPos * 0.875f + targetViewPos * 0.125f;
}

} // namespace rw
