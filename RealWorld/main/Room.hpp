﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/main/rooms/Room.hpp>

#include <RealWorld/main/settings/KeyBinder.hpp>

/**
 * @brief Is an abstract base class for all rooms of RealWorld.
*/
class Room : public RE::Room {
public:

    Room(size_t name, const RE::RoomDisplaySettings& rds) : RE::Room(name, rds) {}

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
};
