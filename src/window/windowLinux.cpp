#include "window/window.hpp"
#include "Logger/Logger.hpp"

#include <cstring>

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

namespace rtk
{
    Window::Window(uint32_t width, uint32_t height, const char* title) 
        : _display(nullptr), _windowHandle(0), _vkInstance(nullptr), _surface(0), _isOpen(true), _width(width), _height(height)
    {
        Display* dpy = XOpenDisplay(NULL);
        if (!dpy) {
            LOG_ERROR("Failed to open X display");
            _isOpen = false;
            return;
        }

        Bool supported = False;
        XkbSetDetectableAutoRepeat(dpy, True, &supported);

        int screen = DefaultScreen(dpy);
        ::Window root = RootWindow(dpy, screen);

        ::Window win = XCreateSimpleWindow(dpy, root, 0, 0, width, height, 1, BlackPixel(dpy, screen), BlackPixel(dpy, screen));
        XStoreName(dpy, win, title);

        XSelectInput(dpy, win, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask);
        XMapWindow(dpy, win);

        Atom wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

        _display = dpy;
        _windowHandle = win;
        _surface = 0;
        _vkInstance = nullptr;

        for (int i = 0; i < 4; ++i) {
            char path[32];
            snprintf(path, sizeof(path), "/dev/input/js%d", i);
            _joystickFds[i] = open(path, O_RDONLY | O_NONBLOCK);
            if (_joystickFds[i] >= 0) {
                LOG_INFO("Joystick connected");
            }
        }

        LOG_INFO("Linux X11 window created");
    }

    Window::~Window()
    {
        for (int i = 0; i < 4; ++i) {
            if (_joystickFds[i] >= 0) {
                close(_joystickFds[i]);
            }
        }
        if (_display) {
            XDestroyWindow((Display*)_display, _windowHandle);
            XCloseDisplay((Display*)_display);
            LOG_INFO("Linux X11 window destroyed");
        }
    }

    void Window::display(RGB clearColor) {}

    bool Window::pollEvents(rtk::Event& rtkEvent)
    {
        if (!_isOpen || !_display)
            return false;

        Display* dpy = static_cast<Display*>(_display);
        XEvent xEvent;

        memset(rtkEvent._keyReleased, 0, RTK_KEYS_TAB_SIZE);
        memset(rtkEvent._mouseButtonReleased, 0, 3);
        memset(rtkEvent._gamepadButtonReleased, 0, sizeof(rtkEvent._gamepadButtonReleased));

        for (int i = 0; i < 4; ++i) {
            if (_joystickFds[i] < 0) {
                rtkEvent._gamepadConnected[i] = false;
                continue;
            }
            rtkEvent._gamepadConnected[i] = true;
            
            struct js_event jse;
            while (read(_joystickFds[i], &jse, sizeof(jse)) > 0) {
                jse.type &= ~JS_EVENT_INIT;
                if (jse.type == JS_EVENT_BUTTON) {
                    GamepadButton btn = GamepadButton::Unknown;
                    switch (jse.number) {
                        case 0: btn = GamepadButton::A; break;
                        case 1: btn = GamepadButton::B; break;
                        case 2: btn = GamepadButton::X; break;
                        case 3: btn = GamepadButton::Y; break;
                        case 4: btn = GamepadButton::L1; break;
                        case 5: btn = GamepadButton::R1; break;
                        case 6: btn = GamepadButton::Select; break;
                        case 7: btn = GamepadButton::Start; break;
                        case 8: btn = GamepadButton::Home; break;
                        case 9: btn = GamepadButton::L3; break;
                        case 10: btn = GamepadButton::R3; break;
                    }
                    if (btn != GamepadButton::Unknown) {
                        bool pressed = (jse.value == 1);
                        if (pressed) {
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(btn)] = true;
                        } else {
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(btn)] = false;
                            rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(btn)] = true;
                        }
                    }
                } else if (jse.type == JS_EVENT_AXIS) {
                    if (jse.number == 6) {
                        if (jse.value > 0) { rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadRight)] = true; rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadLeft)] = false; }
                        else if (jse.value < 0) { rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadLeft)] = true; rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadRight)] = false; }
                        else { 
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadLeft)] = false; 
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadRight)] = false; 
                            rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(GamepadButton::DpadLeft)] = true;
                            rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(GamepadButton::DpadRight)] = true;
                        }
                    } else if (jse.number == 7) {
                        if (jse.value > 0) { rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadDown)] = true; rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadUp)] = false; }
                        else if (jse.value < 0) { rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadUp)] = true; rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadDown)] = false; }
                        else { 
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadUp)] = false; 
                            rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(GamepadButton::DpadDown)] = false; 
                            rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(GamepadButton::DpadUp)] = true;
                            rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(GamepadButton::DpadDown)] = true;
                        }
                    } else {
                        GamepadAxis axisType = GamepadAxis::AxisCount;
                        switch (jse.number) {
                            case 0: axisType = GamepadAxis::LeftX; break;
                            case 1: axisType = GamepadAxis::LeftY; break;
                            case 2: axisType = GamepadAxis::L2; break;
                            case 3: axisType = GamepadAxis::RightX; break;
                            case 4: axisType = GamepadAxis::RightY; break;
                            case 5: axisType = GamepadAxis::R2; break;
                        }
                        if (axisType != GamepadAxis::AxisCount) {
                            float val = static_cast<float>(jse.value) / 32767.0f;
                            rtkEvent._gamepadAxis[i][static_cast<std::size_t>(axisType)] = val;
                        }
                    }
                }
            }
        }

        while (XPending(dpy) > 0) {
            XNextEvent(dpy, &xEvent);

            switch (xEvent.type) {
                case FocusOut: {
                    memset(rtkEvent._keyPressed, 0, RTK_KEYS_TAB_SIZE);
                    memset(rtkEvent._mouseButtonPressed, 0, 3);
                    break;
                }

                case ConfigureNotify: {
                    setWindowSize(xEvent.xconfigure.width, xEvent.xconfigure.height);
                    break;
                }

                case ClientMessage:
                    if (static_cast<Atom>(xEvent.xclient.data.l[0]) ==
                        XInternAtom(dpy, "WM_DELETE_WINDOW", False)) {
                        _isOpen = false;
                    }
                    break;

                case DestroyNotify:
                    _isOpen = false;
                    break;

                case KeyPress: {
                    const unsigned int keycode = xEvent.xkey.keycode;

                    if (keycode < RTK_KEYS_TAB_SIZE)
                        rtkEvent._keyPressed[keycode] = true;

                    break;
                }

                case KeyRelease: {
                    const unsigned int keycode = xEvent.xkey.keycode;

                    if (keycode < RTK_KEYS_TAB_SIZE) {
                        rtkEvent._keyPressed[keycode] = false;
                        rtkEvent._keyReleased[keycode] = true;
                    }

                    break;
                }

                case MotionNotify: {
                    rtkEvent._mouseX = xEvent.xmotion.x;
                    rtkEvent._mouseY = xEvent.xmotion.y;
                    break;
                }

                case ButtonPress: {
                    if (xEvent.xbutton.button == Button1) rtkEvent._mouseButtonPressed[0] = true;
                    else if (xEvent.xbutton.button == Button3) rtkEvent._mouseButtonPressed[1] = true;
                    else if (xEvent.xbutton.button == Button2) rtkEvent._mouseButtonPressed[2] = true;
                    break;
                }

                case ButtonRelease: {
                    if (xEvent.xbutton.button == Button1) {
                        rtkEvent._mouseButtonPressed[0] = false;
                        rtkEvent._mouseButtonReleased[0] = true;
                    }
                    else if (xEvent.xbutton.button == Button3) {
                        rtkEvent._mouseButtonPressed[1] = false;
                        rtkEvent._mouseButtonReleased[1] = true;
                    }
                    else if (xEvent.xbutton.button == Button2) {
                        rtkEvent._mouseButtonPressed[2] = false;
                        rtkEvent._mouseButtonReleased[2] = true;
                    }
                    break;
                }
            }
        }

        return _isOpen;
    }

    std::vector<const char*> Window::getRequiredExtensions() const
    {
        return { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME };
    }

    void Window::createSurface(void *vkInstance)
    {
        _vkInstance = vkInstance;
        VkXlibSurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.dpy = (Display*)_display;
        createInfo.window = (::Window)_windowHandle;

        VkSurfaceKHR surface;
        if (vkCreateXlibSurfaceKHR((VkInstance)vkInstance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
            LOG_ERROR("Failed to create Xlib Vulkan surface");
        } else {
            _surface = surface;
            LOG_INFO("Xlib Vulkan surface created");
        }
    }

    VkSurfaceKHR Window::getSurface() const { return _surface; }
}
