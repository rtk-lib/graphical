#include "event.hpp"

bool rtk::Event::translateKey(rtk::Key key) const
{
    #if defined(__linux__)
        static Display* display = XOpenDisplay(NULL);

        if (!display)
            return false;
    #endif
    switch (key) {
        case rtk::Key::A: return _keyPressed[KEY_A];
        case rtk::Key::B: return _keyPressed[KEY_B];
        case rtk::Key::C: return _keyPressed[KEY_C];
        case rtk::Key::D: return _keyPressed[KEY_D];
        case rtk::Key::E: return _keyPressed[KEY_E];
        case rtk::Key::F: return _keyPressed[KEY_F];
        case rtk::Key::G: return _keyPressed[KEY_G];
        case rtk::Key::H: return _keyPressed[KEY_H];
        case rtk::Key::I: return _keyPressed[KEY_I];
        case rtk::Key::J: return _keyPressed[KEY_J];
        case rtk::Key::K: return _keyPressed[KEY_K];
        case rtk::Key::L: return _keyPressed[KEY_L];
        case rtk::Key::M: return _keyPressed[KEY_M];
        case rtk::Key::N: return _keyPressed[KEY_N];
        case rtk::Key::O: return _keyPressed[KEY_O];
        case rtk::Key::P: return _keyPressed[KEY_P];
        case rtk::Key::Q: return _keyPressed[KEY_Q];
        case rtk::Key::R: return _keyPressed[KEY_R];
        case rtk::Key::S: return _keyPressed[KEY_S];
        case rtk::Key::T: return _keyPressed[KEY_T];
        case rtk::Key::U: return _keyPressed[KEY_U];
        case rtk::Key::V: return _keyPressed[KEY_V];
        case rtk::Key::W: return _keyPressed[KEY_W];
        case rtk::Key::X: return _keyPressed[KEY_X];
        case rtk::Key::Y: return _keyPressed[KEY_Y];
        case rtk::Key::Z: return _keyPressed[KEY_Z];

        case rtk::Key::Num0: return _keyPressed[KEY_NUM0];
        case rtk::Key::Num1: return _keyPressed[KEY_NUM1];
        case rtk::Key::Num2: return _keyPressed[KEY_NUM2];
        case rtk::Key::Num3: return _keyPressed[KEY_NUM3];
        case rtk::Key::Num4: return _keyPressed[KEY_NUM4];
        case rtk::Key::Num5: return _keyPressed[KEY_NUM5];
        case rtk::Key::Num6: return _keyPressed[KEY_NUM6];
        case rtk::Key::Num7: return _keyPressed[KEY_NUM7];
        case rtk::Key::Num8: return _keyPressed[KEY_NUM8];
        case rtk::Key::Num9: return _keyPressed[KEY_NUM9];

        case rtk::Key::Escape:   return _keyPressed[KEY_ESCAPE];
        case rtk::Key::LControl: return _keyPressed[KEY_LCONTROL];
        case rtk::Key::LShift:   return _keyPressed[KEY_LSHIFT];
        case rtk::Key::LAlt:     return _keyPressed[KEY_LALT];
        case rtk::Key::LSystem:  return _keyPressed[KEY_LSYSTEM];
        case rtk::Key::RControl: return _keyPressed[KEY_RCONTROL];
        case rtk::Key::RShift:   return _keyPressed[KEY_RSHIFT];
        case rtk::Key::RAlt:     return _keyPressed[KEY_RALT];
        case rtk::Key::RSystem:  return _keyPressed[KEY_RSYSTEM];
        case rtk::Key::Menu:     return _keyPressed[KEY_MENU];

        case rtk::Key::LBracket:  return _keyPressed[KEY_LBRACKET];
        case rtk::Key::RBracket:  return _keyPressed[KEY_RBRACKET];
        case rtk::Key::Semicolon: return _keyPressed[KEY_SEMICOLON];
        case rtk::Key::Comma:     return _keyPressed[KEY_COMMA];
        case rtk::Key::Period:    return _keyPressed[KEY_PERIOD];
        case rtk::Key::Quote:     return _keyPressed[KEY_QUOTE];
        case rtk::Key::Slash:     return _keyPressed[KEY_SLASH];
        case rtk::Key::Backslash: return _keyPressed[KEY_BACKSLASH];
        case rtk::Key::Tilde:     return _keyPressed[KEY_TILDE];
        case rtk::Key::Equal:     return _keyPressed[KEY_EQUAL];
        case rtk::Key::Hyphen:    return _keyPressed[KEY_HYPHEN];

        case rtk::Key::Space:     return _keyPressed[KEY_SPACE];
        case rtk::Key::Enter:     return _keyPressed[KEY_ENTER];
        case rtk::Key::Backspace: return _keyPressed[KEY_BACKSPACE];
        case rtk::Key::Tab:       return _keyPressed[KEY_TAB];

        case rtk::Key::PageUp:   return _keyPressed[KEY_PAGEUP];
        case rtk::Key::PageDown: return _keyPressed[KEY_PAGEDOWN];
        case rtk::Key::End:      return _keyPressed[KEY_END];
        case rtk::Key::Home:     return _keyPressed[KEY_HOME];
        case rtk::Key::Insert:   return _keyPressed[KEY_INSERT];
        case rtk::Key::Delete:   return _keyPressed[KEY_DELETE];

        case rtk::Key::Add:      return _keyPressed[KEY_ADD];
        case rtk::Key::Subtract: return _keyPressed[KEY_SUBTRACT];
        case rtk::Key::Multiply: return _keyPressed[KEY_MULTIPLY];
        case rtk::Key::Divide:   return _keyPressed[KEY_DIVIDE];

        case rtk::Key::Left:  return _keyPressed[KEY_LEFT];
        case rtk::Key::Right: return _keyPressed[KEY_RIGHT];
        case rtk::Key::Up:    return _keyPressed[KEY_UP];
        case rtk::Key::Down:  return _keyPressed[KEY_DOWN];

        case rtk::Key::Numpad0: return _keyPressed[KEY_NUMPAD0];
        case rtk::Key::Numpad1: return _keyPressed[KEY_NUMPAD1];
        case rtk::Key::Numpad2: return _keyPressed[KEY_NUMPAD2];
        case rtk::Key::Numpad3: return _keyPressed[KEY_NUMPAD3];
        case rtk::Key::Numpad4: return _keyPressed[KEY_NUMPAD4];
        case rtk::Key::Numpad5: return _keyPressed[KEY_NUMPAD5];
        case rtk::Key::Numpad6: return _keyPressed[KEY_NUMPAD6];
        case rtk::Key::Numpad7: return _keyPressed[KEY_NUMPAD7];
        case rtk::Key::Numpad8: return _keyPressed[KEY_NUMPAD8];
        case rtk::Key::Numpad9: return _keyPressed[KEY_NUMPAD9];

        case rtk::Key::F1:  return _keyPressed[KEY_F1];
        case rtk::Key::F2:  return _keyPressed[KEY_F2];
        case rtk::Key::F3:  return _keyPressed[KEY_F3];
        case rtk::Key::F4:  return _keyPressed[KEY_F4];
        case rtk::Key::F5:  return _keyPressed[KEY_F5];
        case rtk::Key::F6:  return _keyPressed[KEY_F6];
        case rtk::Key::F7:  return _keyPressed[KEY_F7];
        case rtk::Key::F8:  return _keyPressed[KEY_F8];
        case rtk::Key::F9:  return _keyPressed[KEY_F9];
        case rtk::Key::F10: return _keyPressed[KEY_F10];
        case rtk::Key::F11: return _keyPressed[KEY_F11];
        case rtk::Key::F12: return _keyPressed[KEY_F12];
        case rtk::Key::F13: return _keyPressed[KEY_F13];
        case rtk::Key::F14: return _keyPressed[KEY_F14];
        case rtk::Key::F15: return _keyPressed[KEY_F15];

        default: return false;
    }
}