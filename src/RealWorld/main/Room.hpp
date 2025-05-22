/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <utility>

#include <RealEngine/rooms/Room.hpp>

#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/main/settings/KeyBinder.hpp>

namespace rw {

enum class RoomName {
    MainMenu,
    World
};

/**
 * @brief Is a base class for all rooms of RealWorld.
 */
class Room: public re::Room {
public:
    Room(RoomName name, const re::RoomDisplaySettings& rds)
        : re::Room{static_cast<size_t>(name), rds} {}

    template<typename RoomType, typename... Args>
    void scheduleTransition(Args&&... transitionArgs) {
        engine().scheduleRoomTransition(
            static_cast<size_t>(RoomType::k_name),
            {std::make_any<typename RoomType::TransitionArgs>(std::forward<Args>(transitionArgs
            )...)}
        );
    }

protected:
    using enum RealWorldKeyBindings;

    auto keybindPressed(RealWorldKeyBindings binding) {
        return engine().wasKeyPressed(keybinder(binding));
    }

    auto keybindReleased(RealWorldKeyBindings binding) {
        return engine().wasKeyReleased(keybinder(binding));
    }

    auto keybindDown(RealWorldKeyBindings binding) {
        return engine().isKeyDown(keybinder(binding));
    }

    template<re::CompTimeString k_lit>
    ImFont* createFont(float size) const {
        std::vector<unsigned char> vec = re::RM::dataUnmanaged(fontID<k_lit>());
        auto* ptr = static_cast<unsigned char*>(IM_ALLOC(vec.size()));
        std::memcpy(ptr, vec.data(), vec.size());
        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            ptr, static_cast<int>(vec.size()), size
        );
        // ptr will be freed by ImGui
    }
};

} // namespace rw
