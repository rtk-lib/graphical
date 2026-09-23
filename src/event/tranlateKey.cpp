#include "event/event.hpp"

std::size_t rtk::Event::translateKey(rtk::Key key) const
{
    #if defined(__linux__)
        static Display* display = XOpenDisplay(NULL);

        if (!display)
            return RTK_KEYS_TAB_SIZE;
    #endif
    switch (key) {
        case rtk::Key::A: return KEY_A;
        case rtk::Key::B: return KEY_B;
        case rtk::Key::C: return KEY_C;
        case rtk::Key::D: return KEY_D;
        case rtk::Key::E: return KEY_E;
        case rtk::Key::F: return KEY_F;
        case rtk::Key::G: return KEY_G;
        case rtk::Key::H: return KEY_H;
        case rtk::Key::I: return KEY_I;
        case rtk::Key::J: return KEY_J;
        case rtk::Key::K: return KEY_K;
        case rtk::Key::L: return KEY_L;
        case rtk::Key::M: return KEY_M;
        case rtk::Key::N: return KEY_N;
        case rtk::Key::O: return KEY_O;
        case rtk::Key::P: return KEY_P;
        case rtk::Key::Q: return KEY_Q;
        case rtk::Key::R: return KEY_R;
        case rtk::Key::S: return KEY_S;
        case rtk::Key::T: return KEY_T;
        case rtk::Key::U: return KEY_U;
        case rtk::Key::V: return KEY_V;
        case rtk::Key::W: return KEY_W;
        case rtk::Key::X: return KEY_X;
        case rtk::Key::Y: return KEY_Y;
        case rtk::Key::Z: return KEY_Z;

        case rtk::Key::Num0: return KEY_NUM0;
        case rtk::Key::Num1: return KEY_NUM1;
        case rtk::Key::Num2: return KEY_NUM2;
        case rtk::Key::Num3: return KEY_NUM3;
        case rtk::Key::Num4: return KEY_NUM4;
        case rtk::Key::Num5: return KEY_NUM5;
        case rtk::Key::Num6: return KEY_NUM6;
        case rtk::Key::Num7: return KEY_NUM7;
        case rtk::Key::Num8: return KEY_NUM8;
        case rtk::Key::Num9: return KEY_NUM9;

        case rtk::Key::Escape:   return KEY_ESCAPE;
        case rtk::Key::LControl: return KEY_LCONTROL;
        case rtk::Key::LShift:   return KEY_LSHIFT;
        case rtk::Key::LAlt:     return KEY_LALT;
        case rtk::Key::LSystem:  return KEY_LSYSTEM;
        case rtk::Key::RControl: return KEY_RCONTROL;
        case rtk::Key::RShift:   return KEY_RSHIFT;
        case rtk::Key::RAlt:     return KEY_RALT;
        case rtk::Key::RSystem:  return KEY_RSYSTEM;
        case rtk::Key::Menu:     return KEY_MENU;

        case rtk::Key::LBracket:  return KEY_LBRACKET;
        case rtk::Key::RBracket:  return KEY_RBRACKET;
        case rtk::Key::Semicolon: return KEY_SEMICOLON;
        case rtk::Key::Comma:     return KEY_COMMA;
        case rtk::Key::Period:    return KEY_PERIOD;
        case rtk::Key::Quote:     return KEY_QUOTE;
        case rtk::Key::Slash:     return KEY_SLASH;
        case rtk::Key::Backslash: return KEY_BACKSLASH;
        case rtk::Key::Tilde:     return KEY_TILDE;
        case rtk::Key::Equal:     return KEY_EQUAL;
        case rtk::Key::Hyphen:    return KEY_HYPHEN;

        case rtk::Key::Space:     return KEY_SPACE;
        case rtk::Key::Enter:     return KEY_ENTER;
        case rtk::Key::Backspace: return KEY_BACKSPACE;
        case rtk::Key::Tab:       return KEY_TAB;

        case rtk::Key::PageUp:   return KEY_PAGEUP;
        case rtk::Key::PageDown: return KEY_PAGEDOWN;
        case rtk::Key::End:      return KEY_END;
        case rtk::Key::Home:     return KEY_HOME;
        case rtk::Key::Insert:   return KEY_INSERT;
        case rtk::Key::Delete:   return KEY_DELETE;

        case rtk::Key::Add:      return KEY_ADD;
        case rtk::Key::Subtract: return KEY_SUBTRACT;
        case rtk::Key::Multiply: return KEY_MULTIPLY;
        case rtk::Key::Divide:   return KEY_DIVIDE;

        case rtk::Key::Left:  return KEY_LEFT;
        case rtk::Key::Right: return KEY_RIGHT;
        case rtk::Key::Up:    return KEY_UP;
        case rtk::Key::Down:  return KEY_DOWN;

        case rtk::Key::Numpad0: return KEY_NUMPAD0;
        case rtk::Key::Numpad1: return KEY_NUMPAD1;
        case rtk::Key::Numpad2: return KEY_NUMPAD2;
        case rtk::Key::Numpad3: return KEY_NUMPAD3;
        case rtk::Key::Numpad4: return KEY_NUMPAD4;
        case rtk::Key::Numpad5: return KEY_NUMPAD5;
        case rtk::Key::Numpad6: return KEY_NUMPAD6;
        case rtk::Key::Numpad7: return KEY_NUMPAD7;
        case rtk::Key::Numpad8: return KEY_NUMPAD8;
        case rtk::Key::Numpad9: return KEY_NUMPAD9;

        case rtk::Key::F1:  return KEY_F1;
        case rtk::Key::F2:  return KEY_F2;
        case rtk::Key::F3:  return KEY_F3;
        case rtk::Key::F4:  return KEY_F4;
        case rtk::Key::F5:  return KEY_F5;
        case rtk::Key::F6:  return KEY_F6;
        case rtk::Key::F7:  return KEY_F7;
        case rtk::Key::F8:  return KEY_F8;
        case rtk::Key::F9:  return KEY_F9;
        case rtk::Key::F10: return KEY_F10;
        case rtk::Key::F11: return KEY_F11;
        case rtk::Key::F12: return KEY_F12;
        case rtk::Key::F13: return KEY_F13;
        case rtk::Key::F14: return KEY_F14;
        case rtk::Key::F15: return KEY_F15;

        default: return RTK_KEYS_TAB_SIZE;
    }
}