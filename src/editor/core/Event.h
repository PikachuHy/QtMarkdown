#ifndef QTMARKDOWN_CORE_EVENT_H
#define QTMARKDOWN_CORE_EVENT_H

#include "Types.h"
#include <string>

namespace md::editor::core {

// -- Modifier keys (bitmask) --
enum class Modifier : int {
    None      = 0,
    Ctrl      = 1 << 0,
    Shift     = 1 << 1,
    Alt       = 1 << 2,
    Meta      = 1 << 3,
    Keypad    = 1 << 4,
    GroupSwitch = 1 << 5,
};
constexpr Modifier operator|(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<int>(a) | static_cast<int>(b));
}
constexpr Modifier operator&(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<int>(a) & static_cast<int>(b));
}

// -- Key codes (platform-independent subset) --
enum class Key : int {
    Unknown      = 0,
    Tab          = 1,
    Escape       = 2,
    Return       = 3,
    Backspace    = 4,
    Left         = 1000,
    Right        = 1001,
    Up           = 1002,
    Down         = 1003,
    // Modifier keys -- added here in Phase 1 for completeness in the enum;
    // the adapter maps them in Phase 3.
    Key_Control  = 1004,
    Key_Shift    = 1005,
    A = 'A', B = 'B', C = 'C', D = 'D', E = 'E',
    F = 'F', G = 'G', H = 'H', I = 'I', J = 'J',
    K = 'K', L = 'L', M = 'M', N = 'N', O = 'O',
    P = 'P', Q = 'Q', R = 'R', S = 'S', T = 'T',
    U = 'U', V = 'V', W = 'W', X = 'X', Y = 'Y',
    Z = 'Z',
    Key_1 = '1', Key_2 = '2', Key_3 = '3', Key_4 = '4',
    Key_5 = '5', Key_6 = '6', Key_7 = '7', Key_8 = '8',
    Key_9 = '9', Key_0 = '0',
};

// -- Mouse buttons --
enum class MouseButton : int {
    None   = 0,
    Left   = 1,
    Right  = 2,
    Middle = 4,
};

// -- Mouse event types --
enum class MouseEventType : int {
    Press,
    Move,
    Release,
};

// -- Abstract key event --
class KeyEvent {
public:
    virtual ~KeyEvent() = default;
    virtual Key key() const = 0;
    virtual Modifier modifiers() const = 0;
    virtual std::string text() const = 0;
    virtual bool isAutoRepeat() const = 0;
};

// -- Abstract mouse event --
class MouseEvent {
public:
    virtual ~MouseEvent() = default;
    virtual MouseEventType type() const = 0;
    virtual Point pos() const = 0;
    virtual MouseButton button() const = 0;
    virtual Modifier modifiers() const = 0;
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_EVENT_H
