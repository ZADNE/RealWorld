/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/user_input/KeyBinder.hpp>

namespace rw {

/**
 * @brief Represents all key bindings that RealWorld has
 */
enum class RealWorldKeyBindings {
    InvOpenClose,
    InvMoveAll,
    InvMovePortion,
    InvPrevSlot,
    InvRightSlot,
    InvLeftSlot,
    InvSlot0,
    InvSlot1,
    InvSlot2,
    InvSlot3,
    InvSlot4,
    InvSlot5,
    InvSlot6,
    InvSlot7,
    InvSlot8,
    InvSlot9,

    ItemuserUsePrimary,
    ItemuserUseSecondary,
    ItemuserSwitchShape,
    ItemuserHoldToResize,
    ItemuserWiden,
    ItemuserShrink,

    PlayerLeft,
    PlayerRight,
    PlayerJump,
    PlayerAutojump,

    Quit,
    Minimap,
    Shadows,
    Permute,

    Count
};

struct BindingInfo {
    constexpr BindingInfo(re::Key defaultValue, const char* name, const char* desc)
        : defaultValue(defaultValue)
        , name(name)
        , desc(desc) {}

    re::Key     defaultValue;
    const char* name;
    const char* desc;
};

constexpr static std::array<BindingInfo, static_cast<size_t>(RealWorldKeyBindings::Count)> k_keybindingInfo = {
    BindingInfo{re::Key::E,         "INV_OPEN_CLOSE",           "Open/close"},
    BindingInfo{re::Key::LMB,       "INV_MOVE_ALL",             "Take/put all"},
    BindingInfo{re::Key::RMB,       "INV_MOVE_PORTION",         "Take/put some"},
    BindingInfo{re::Key::Q,         "INV_PREV_SLOT",            "ToPrevious slot"},
    BindingInfo{re::Key::DMW,       "INV_RIGHT_SLOT",           "ScrollLeft slot"},
    BindingInfo{re::Key::UMW,       "INV_LEFT_SLOT",            "ScrollRight slot"},
    BindingInfo{re::Key::K1,        "INV_SLOT0",                "Slot 1"},
    BindingInfo{re::Key::K2,        "INV_SLOT1",                "Slot 2"},
    BindingInfo{re::Key::K3,        "INV_SLOT2",                "Slot 3"},
    BindingInfo{re::Key::K4,        "INV_SLOT3",                "Slot 4"},
    BindingInfo{re::Key::K5,        "INV_SLOT4",                "Slot 5"},
    BindingInfo{re::Key::K6,        "INV_SLOT5",                "Slot 6"},
    BindingInfo{re::Key::K7,        "INV_SLOT6",                "Slot 7"},
    BindingInfo{re::Key::K8,        "INV_SLOT7",                "Slot 8"},
    BindingInfo{re::Key::K9,        "INV_SLOT8",                "Slot 9"},
    BindingInfo{re::Key::K0,        "INV_SLOT9",                "Slot 10"},

    BindingInfo{re::Key::LMB,       "ITEMUSER_USE_PRIMARY",     "Primary"},
    BindingInfo{re::Key::RMB,       "ITEMUSER_USE_SECONDARY",   "Secondary"},
    BindingInfo{re::Key::MMB,       "ITEMUSER_SWITCH_SHAPE",    "Disk/square"},
    BindingInfo{re::Key::LCtrl,     "ITEMUSER_HOLD_TO_RESIZE",  "Hold to resize"},
    BindingInfo{re::Key::UMW,       "ITEMUSER_WIDEN",           "Widen"},
    BindingInfo{re::Key::DMW,       "ITEMUSER_SHRINK",          "Shrink"},

    BindingInfo{re::Key::A,         "PLAYER_LEFT",              "Walk left"},
    BindingInfo{re::Key::D,         "PLAYER_RIGHT",             "Walk right"},
    BindingInfo{re::Key::Space,     "PLAYER_JUMP",              "Jump"},
    BindingInfo{re::Key::LShift,    "PLAYER_AUTOJUMP",          "Autojump"},

    BindingInfo{re::Key::Escape,    "QUIT",                     "Quit"},
    BindingInfo{re::Key::Numpad1,   "MINIMAP",                  "Draw minimap"},
    BindingInfo{re::Key::Numpad2,   "SHADOWS",                  "Draw shadows"},
    BindingInfo{re::Key::Numpad3,   "PERMUTE",                  "Permute order"}
};

/**
 * @brief Global keybinder object for the RealWorld game
 */
inline auto& keybinder() {
    static re::KeyBinder<RealWorldKeyBindings, BindingInfo, k_keybindingInfo> kb{};
    return kb;
}

/**
 * @brief Shortcut for keybinder()[binding]
 */
inline re::Key keybinder(RealWorldKeyBindings binding) {
    return keybinder()[binding];
}

} // namespace rw