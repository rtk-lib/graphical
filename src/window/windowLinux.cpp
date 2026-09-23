#include "window/window.hpp"
#include "Logger/Logger.hpp"

#include <cstring>

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

namespace rtk
{
    Window::Window(uint32_t width, uint32_t height, const char* title) : _isOpen(true), _width(width), _height(height)
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

        LOG_INFO("Linux X11 window created");
    }

    Window::~Window()
    {
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

        while (XPending(dpy) > 0) {
            XNextEvent(dpy, &xEvent);

            switch (xEvent.type) {
                case FocusOut: {
                    memset(rtkEvent._keyPressed, 0, RTK_KEYS_TAB_SIZE);
                    memset(rtkEvent._mouseButtonPressed, 0, 3);
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
