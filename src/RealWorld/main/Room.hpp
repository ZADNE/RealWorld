﻿/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rooms/Room.hpp>

#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/main/settings/KeyBinder.hpp>

namespace rw {

/**
 * @brief Is an abstract base class for all rooms of RealWorld.
 */
class Room: public re::Room {
public:
    Room(size_t name, const re::RoomDisplaySettings& rds)
        : re::Room(name, rds) {}

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
