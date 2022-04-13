#pragma once
#include <RealEngine/main/room/Room.hpp>

#include <RealWorld/main/KeyBinder.hpp>

/**
 * @brief Abstract base class for all rooms of RealWorld.
*/
class Room : public RE::Room {
public:
	Room() {}
	virtual ~Room() {}

protected:
	using enum RealWorldKeyBindings;

	int keybindPressed(RealWorldKeyBindings binding) {
		return input()->wasPressed(keybinder(binding));
	}

	int keybindReleased(RealWorldKeyBindings binding) {
		return input()->wasReleased(keybinder(binding));
	}

	int keybindDown(RealWorldKeyBindings binding) {
		return input()->isDown(keybinder(binding));
	}
};
