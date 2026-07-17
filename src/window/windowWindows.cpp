/*
 * Windows (Win32) Window implementation
 */
#include "window.hpp"
#include "../Logger/Logger.hpp"
#include <windows.h>

namespace rtk 
{
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        rtk::Window* win = nullptr;
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            win = reinterpret_cast<rtk::Window*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)win);
        } else {
            win = reinterpret_cast<rtk::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        switch (uMsg) {
            case WM_CLOSE:
                if (win) {
                    PostQuitMessage(0);
                }
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Window::Window(uint32_t width, uint32_t height, const char* title) : _isOpen(true)
    {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        const char* CLASS_NAME = "rtk_window_class";

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(
            0,
            CLASS_NAME,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL,
            NULL,
            hInstance,
            this
        );

        if (!hwnd) {
            LOG_ERROR("Failed to create Win32 window");
            _isOpen = false;
            return;
        }

        ShowWindow(hwnd, SW_SHOW);
        _display = hInstance;
        _windowHandle = reinterpret_cast<uint64_t>(hwnd);
        _surface = 0;
        _vkInstance = nullptr;

        LOG_INFO("Windows Win32 window created");
    }

    Window::~Window()
    {
        if (_windowHandle) {
            DestroyWindow(reinterpret_cast<HWND>(_windowHandle));
            LOG_INFO("Windows Win32 window destroyed");
        }
    }

    void Window::display(RGB clearColor)
    {
    }

    uint64_t Window::getSurface() const
    {
        return _surface;
    }

    bool Window::pollEvents()
    {
        if (!_isOpen) return false;
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                _isOpen = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return _isOpen;
    }
}
