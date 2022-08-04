/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/main/room/Room.hpp>

#include <RealWorld/main/settings/KeyBinder.hpp>

 /**
  * @brief Is an abstract base class for all rooms of RealWorld.
 */
class Room : public RE::Room {
public:
	Room() {}

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
