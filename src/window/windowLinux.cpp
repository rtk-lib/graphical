#include "window.hpp"
#include "../Logger/Logger.hpp"

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace rtk 
{
    Window::Window(uint32_t width, uint32_t height, const char* title) : _isOpen(true)
    {
        Display* dpy = XOpenDisplay(NULL);
        if (!dpy) {
            LOG_ERROR("Failed to open X display");
            _isOpen = false;
            return;
        }

        int screen = DefaultScreen(dpy);
        ::Window root = RootWindow(dpy, screen);

        ::Window win = XCreateSimpleWindow(dpy, root, 0, 0, width, height, 1, BlackPixel(dpy, screen), WhitePixel(dpy, screen));
        XStoreName(dpy, win, title);

        XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);
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

    bool Window::pollEvents()
    {
        if (!_isOpen || !_display) return false;
        Display* dpy = (Display*)_display;
        XEvent event;
        while (XPending(dpy) > 0) {
            XNextEvent(dpy, &event);
            if (event.type == ClientMessage) {
                if ((Atom)event.xclient.data.l[0] == XInternAtom(dpy, "WM_DELETE_WINDOW", False)) {
                    _isOpen = false;
                }
            } else if (event.type == DestroyNotify) {
                _isOpen = false;
            }
        }
        return _isOpen;
    }

    std::vector<const char*> Window::getRequiredExtensions() const
    {
        return { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME };
    }

    void Window::createSurface(void* vkInstance)
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
            _surface = (uint64_t)surface;
            LOG_INFO("Xlib Vulkan surface created");
        }
    }

    uint64_t Window::getSurface() const { return _surface; }
}
