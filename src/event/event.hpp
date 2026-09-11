#pragma once

#include "../fwd.hpp"

#define RTK_KEYS_TAB_SIZE 256

namespace rtk {

    class Event
    {
        private:
            bool _keyPressed[RTK_KEYS_TAB_SIZE];
            bool _keyReleased[RTK_KEYS_TAB_SIZE];
        public:
            /**
             *  @brief make Window a friend of Event
             *  to let him fill the key
             */
            friend class Window;
    };

    /**
     * @brief Enumeration of all supported keyboard keys.
     * Abstracted cross-platform key codes used by the ECS and Event system.
     */
    enum class Key : uint16_t
    {
        Unknown = 0,

        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        Escape,
        LControl, LShift, LAlt, LSystem,
        RControl, RShift, RAlt, RSystem,
        Menu,

        LBracket, RBracket, Semicolon, Comma, Period, Quote, Slash,
        Backslash, Tilde, Equal, Hyphen,

        Space, Enter, Backspace, Tab,
        PageUp, PageDown, End, Home, Insert, Delete,

        Add, Subtract, Multiply, Divide,

        Left, Right, Up, Down,

        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
    };
}
