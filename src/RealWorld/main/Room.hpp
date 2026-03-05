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

class Room;

/**
 * @brief All rooms of RealWorld should adhere to this
 */
template<class T>
concept RealWorldRoom = requires(T a) {
    requires std::derived_from<T, Room>;
    requires std::is_same_v<decltype(T::k_name), const RoomName>;
    typename T::TransitionArgs;
};

/**
 * @brief   Is a base class for all rooms of RealWorld.
 * @details Derived rooms should adhere to RealWorldRoom concept.
 */
class Room: public re::Room {
public:
    Room(RoomName name, const re::RoomDisplaySettings& rds)
        : re::Room{static_cast<size_t>(name), rds} {}

    template<RealWorldRoom RoomType, typename... Args>
    void scheduleTransition(Args&&... transitionArgs) {
        engine().scheduleRoomTransition(
            static_cast<size_t>(RoomType::k_name),
            {std::make_any<typename RoomType::TransitionArgs>(
                std::forward<Args>(transitionArgs)...
            )}
        );
    }

protected:
    using enum KeyBinding;

    auto keybindPressed(KeyBinding binding) {
        return engine().wasKeyPressed(keyBinder(binding));
    }

    auto keybindReleased(KeyBinding binding) {
        return engine().wasKeyReleased(keyBinder(binding));
    }

    auto keybindDown(KeyBinding binding) {
        return engine().isKeyDown(keyBinder(binding));
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
