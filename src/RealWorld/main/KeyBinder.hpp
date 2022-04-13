#pragma once
#include <RealEngine/user_input/KeyBinder.hpp>

/**
 * @brief Represents all key bindings that RealWorld has
*/
enum class RealWorldKeyBindings {
	INV_OPEN_CLOSE,
	INV_MOVE_ALL,
	INV_MOVE_PORTION,
	INV_PREV_SLOT,
	INV_RIGHT_SLOT,
	INV_LEFT_SLOT,
	INV_SLOT0,
	INV_SLOT1,
	INV_SLOT2,
	INV_SLOT3,
	INV_SLOT4,
	INV_SLOT5,
	INV_SLOT6,
	INV_SLOT7,
	INV_SLOT8,
	INV_SLOT9,

	ITEMUSER_USE_PRIMARY,
	ITEMUSER_USE_SECONDARY,
	ITEMUSER_SWITCH_SHAPE,
	ITEMUSER_HOLD_TO_RESIZE,
	ITEMUSER_WIDEN,
	ITEMUSER_SHRINK,

	PLAYER_LEFT,
	PLAYER_RIGHT,
	PLAYER_JUMP,
	PLAYER_AUTOJUMP,

	QUIT,
	MINIMAP,
	SHADOWS
};

constexpr static std::array KEYBINDER_DESC = {
	"Open/close",
	"Take/put all",
	"Take/put some",
	"Previous slot",
	"Left slot",
	"Right slot",
	"Slot 1",
	"Slot 2",
	"Slot 3",
	"Slot 4",
	"Slot 5",
	"Slot 6",
	"Slot 7",
	"Slot 8",
	"Slot 9",
	"Slot 10",

	"Primary",
	"Secondary",
	"Disc/square",
	"Hold to resize",
	"Widen",
	"Shrink",

	"Walk left",
	"Walk right",
	"Jump",
	"Autojump",

	"Quit",
	"Draw minimap",
	"Draw shadows"
};

/**
 * @brief Default values for key bindings if saved key bidnings could not be loaded
*/
constexpr static RE::KeyBindingValueList<const RealWorldKeyBindings> KEYBINDER_DEFAULT_LIST = {
	RE::Key::E,
	RE::Key::LMB,
	RE::Key::RMB,
	RE::Key::Q,
	RE::Key::DMW,
	RE::Key::UMW,
	RE::Key::K1,
	RE::Key::K2,
	RE::Key::K3,
	RE::Key::K4,
	RE::Key::K5,
	RE::Key::K6,
	RE::Key::K7,
	RE::Key::K8,
	RE::Key::K9,
	RE::Key::K0,

	RE::Key::LMB,
	RE::Key::RMB,
	RE::Key::MMB,
	RE::Key::LCtrl,
	RE::Key::UMW,
	RE::Key::DMW,

	RE::Key::A,
	RE::Key::D,
	RE::Key::Space,
	RE::Key::LShift,

	RE::Key::Escape,
	RE::Key::P,
	RE::Key::O
};

static_assert(KEYBINDER_DEFAULT_LIST.size() == magic_enum::enum_count<RealWorldKeyBindings>()
			&& KEYBINDER_DESC.size() == magic_enum::enum_count<RealWorldKeyBindings>());

/**
 * @brief Global keybinder object for the RealWorld game
 *
 * Can also be accessed via abbrevation KB macro. KB = keybinder()
*/
inline auto& keybinder() {
	static RE::KeyBinder<RealWorldKeyBindings, KEYBINDER_DEFAULT_LIST> kb{};
	return kb;
}

/**
 * @brief Shortcut for keybinder()[binding]
*/
inline RE::Key keybinder(RealWorldKeyBindings binding) {
	return keybinder()[binding];
}