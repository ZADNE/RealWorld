/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <type_traits>

#include <RealEngine/user_input/KeyBinder.hpp>

namespace rw {

/**
 * @brief Lists all key bindings that RealWorld has
 */
enum class KeyBinding {
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

using KeyBindingIntType = std::underlying_type_t<KeyBinding>;

struct KeyBindingInfo {
    constexpr KeyBindingInfo(re::Key defaultValue, const char* name, const char* desc)
        : defaultValue(defaultValue)
        , name(name)
        , desc(desc) {}

    re::Key defaultValue;
    const char* name;
    const char* desc;
};

using KeyBinder = re::KeyBinder<KeyBinding, KeyBindingInfo>;

/**
 * @brief Global keybinder object for the RealWorld game
 */
KeyBinder& keyBinder();

/**
 * @brief Shortcut for keybinder()[binding]
 */
re::Key keyBinder(KeyBinding binding);
re::Key keyBinder(KeyBindingIntType binding);

const KeyBindingInfo& keyBindingInfo(KeyBinding binding);
const KeyBindingInfo& keyBindingInfo(KeyBindingIntType binding);

} // namespace rw
