/*! 
 *  @author    Dubsky Tomas
 */
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
    SHADOWS,
    PERMUTE,

    COUNT
};

struct BindingInfo {
    constexpr BindingInfo(RE::Key defaultValue, const char* name, const char* desc) :
        defaultValue(defaultValue), name(name), desc(desc) {}

    RE::Key defaultValue;
    const char* name;
    const char* desc;
};

constexpr static std::array<BindingInfo, static_cast<size_t>(RealWorldKeyBindings::COUNT)> KEYBINDING_INFO = {
    BindingInfo{RE::Key::E,         "INV_OPEN_CLOSE",           "Open/close"},
    BindingInfo{RE::Key::LMB,       "INV_MOVE_ALL",             "Take/put all"},
    BindingInfo{RE::Key::RMB,       "INV_MOVE_PORTION",         "Take/put some"},
    BindingInfo{RE::Key::Q,         "INV_PREV_SLOT",            "Previous slot"},
    BindingInfo{RE::Key::DMW,       "INV_RIGHT_SLOT",           "Left slot"},
    BindingInfo{RE::Key::UMW,       "INV_LEFT_SLOT",            "Right slot"},
    BindingInfo{RE::Key::K1,        "INV_SLOT0",                "Slot 1"},
    BindingInfo{RE::Key::K2,        "INV_SLOT1",                "Slot 2"},
    BindingInfo{RE::Key::K3,        "INV_SLOT2",                "Slot 3"},
    BindingInfo{RE::Key::K4,        "INV_SLOT3",                "Slot 4"},
    BindingInfo{RE::Key::K5,        "INV_SLOT4",                "Slot 5"},
    BindingInfo{RE::Key::K6,        "INV_SLOT5",                "Slot 6"},
    BindingInfo{RE::Key::K7,        "INV_SLOT6",                "Slot 7"},
    BindingInfo{RE::Key::K8,        "INV_SLOT7",                "Slot 8"},
    BindingInfo{RE::Key::K9,        "INV_SLOT8",                "Slot 9"},
    BindingInfo{RE::Key::K0,        "INV_SLOT9",                "Slot 10"},

    BindingInfo{RE::Key::LMB,       "ITEMUSER_USE_PRIMARY",     "Primary"},
    BindingInfo{RE::Key::RMB,       "ITEMUSER_USE_SECONDARY",   "Secondary"},
    BindingInfo{RE::Key::MMB,       "ITEMUSER_SWITCH_SHAPE",    "Disc/square"},
    BindingInfo{RE::Key::LCtrl,     "ITEMUSER_HOLD_TO_RESIZE",  "Hold to resize"},
    BindingInfo{RE::Key::UMW,       "ITEMUSER_WIDEN",           "Widen"},
    BindingInfo{RE::Key::DMW,       "ITEMUSER_SHRINK",          "Shrink"},

    BindingInfo{RE::Key::A,         "PLAYER_LEFT",              "Walk left"},
    BindingInfo{RE::Key::D,         "PLAYER_RIGHT",             "Walk right"},
    BindingInfo{RE::Key::Space,     "PLAYER_JUMP",              "Jump"},
    BindingInfo{RE::Key::LShift,    "PLAYER_AUTOJUMP",          "Autojump"},

    BindingInfo{RE::Key::Escape,    "QUIT",                     "Quit"},
    BindingInfo{RE::Key::Numpad1,   "MINIMAP",                  "Draw minimap"},
    BindingInfo{RE::Key::Numpad2,   "SHADOWS",                  "Draw shadows"},
    BindingInfo{RE::Key::Numpad3,   "PERMUTE",                  "Permute order"}
};

/**
 * @brief Global keybinder object for the RealWorld game
*/
inline auto& keybinder() {
    static RE::KeyBinder<RealWorldKeyBindings, BindingInfo, KEYBINDING_INFO> kb{};
    return kb;
}

/**
 * @brief Shortcut for keybinder()[binding]
*/
inline RE::Key keybinder(RealWorldKeyBindings binding) {
    return keybinder()[binding];
}