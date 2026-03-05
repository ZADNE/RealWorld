/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/settings/KeyBinder.hpp>

namespace rw {

// clang-format off: Custom alignment of columns
constexpr std::array<KeyBindingInfo, static_cast<size_t>(KeyBinding::Count)> k_keyBindingInfos = {
    KeyBindingInfo{re::Key::E,         "INV_OPEN_CLOSE",           "Open/close"},
    KeyBindingInfo{re::Key::LMB,       "INV_MOVE_ALL",             "Take/put all"},
    KeyBindingInfo{re::Key::RMB,       "INV_MOVE_PORTION",         "Take/put some"},
    KeyBindingInfo{re::Key::Q,         "INV_PREV_SLOT",            "ToPrevious slot"},
    KeyBindingInfo{re::Key::DMW,       "INV_RIGHT_SLOT",           "ScrollLeft slot"},
    KeyBindingInfo{re::Key::UMW,       "INV_LEFT_SLOT",            "ScrollRight slot"},
    KeyBindingInfo{re::Key::K1,        "INV_SLOT0",                "Slot 1"},
    KeyBindingInfo{re::Key::K2,        "INV_SLOT1",                "Slot 2"},
    KeyBindingInfo{re::Key::K3,        "INV_SLOT2",                "Slot 3"},
    KeyBindingInfo{re::Key::K4,        "INV_SLOT3",                "Slot 4"},
    KeyBindingInfo{re::Key::K5,        "INV_SLOT4",                "Slot 5"},
    KeyBindingInfo{re::Key::K6,        "INV_SLOT5",                "Slot 6"},
    KeyBindingInfo{re::Key::K7,        "INV_SLOT6",                "Slot 7"},
    KeyBindingInfo{re::Key::K8,        "INV_SLOT7",                "Slot 8"},
    KeyBindingInfo{re::Key::K9,        "INV_SLOT8",                "Slot 9"},
    KeyBindingInfo{re::Key::K0,        "INV_SLOT9",                "Slot 10"},

    KeyBindingInfo{re::Key::LMB,       "ITEMUSER_USE_PRIMARY",     "Primary"},
    KeyBindingInfo{re::Key::RMB,       "ITEMUSER_USE_SECONDARY",   "Secondary"},
    KeyBindingInfo{re::Key::MMB,       "ITEMUSER_SWITCH_SHAPE",    "Disk/square"},
    KeyBindingInfo{re::Key::LCtrl,     "ITEMUSER_HOLD_TO_RESIZE",  "Hold to resize"},
    KeyBindingInfo{re::Key::UMW,       "ITEMUSER_WIDEN",           "Widen"},
    KeyBindingInfo{re::Key::DMW,       "ITEMUSER_SHRINK",          "Shrink"},

    KeyBindingInfo{re::Key::A,         "PLAYER_LEFT",              "Walk left"},
    KeyBindingInfo{re::Key::D,         "PLAYER_RIGHT",             "Walk right"},
    KeyBindingInfo{re::Key::Space,     "PLAYER_JUMP",              "Jump"},
    KeyBindingInfo{re::Key::LShift,    "PLAYER_AUTOJUMP",          "Autojump"},

    KeyBindingInfo{re::Key::Escape,    "QUIT",                     "Quit"},
    KeyBindingInfo{re::Key::Numpad1,   "MINIMAP",                  "Draw minimap"},
    KeyBindingInfo{re::Key::Numpad2,   "SHADOWS",                  "Draw shadows"},
    KeyBindingInfo{re::Key::Numpad3,   "PERMUTE",                  "Permute order"}
};
// clang-format on

KeyBinder& keyBinder() {
    static KeyBinder s_kb{k_keyBindingInfos};
    return s_kb;
}

re::Key keyBinder(KeyBinding binding) {
    return keyBinder()[binding];
}

re::Key keyBinder(KeyBindingIntType binding) {
    assert(binding >= 0);
    assert(binding < std::to_underlying(KeyBinding::Count));
    return keyBinder()[static_cast<KeyBinding>(binding)];
}

const KeyBindingInfo& keyBindingInfo(KeyBinding binding) {
    return keyBindingInfo(std::to_underlying(binding));
}

const KeyBindingInfo& keyBindingInfo(KeyBindingIntType binding) {
    return k_keyBindingInfos[binding];
}

} // namespace rw
