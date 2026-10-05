#include "window/window.hpp"
#include "Logger/Logger.hpp"
#include <cstring>
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

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
            case WM_SIZE:
                if (win) {
                    win->setWindowSize(LOWORD(lParam), HIWORD(lParam));
                }
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Window::Window(uint32_t width, uint32_t height, const char* title) 
        : _display(nullptr), _windowHandle(0), _vkInstance(nullptr), _surface(0), _isOpen(true), _width(width), _height(height)
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

    void Window::display(RGB clearColor) {}

    bool Window::pollEvents(rtk::Event& rtkEvent)
    {
        if (!_isOpen) return false;
        MSG msg = {};
        memset(rtkEvent._keyReleased, 0, RTK_KEYS_TAB_SIZE);
        memset(rtkEvent._mouseButtonReleased, 0, 3);
        memset(rtkEvent._gamepadButtonReleased, 0, sizeof(rtkEvent._gamepadButtonReleased));

        for (DWORD i = 0; i < 4; i++) {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));

            if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                rtkEvent._gamepadConnected[i] = true;

                auto checkButton = [&](WORD xinputButton, GamepadButton rtkBtn) {
                    bool pressed = (state.Gamepad.wButtons & xinputButton) != 0;
                    if (pressed && !rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(rtkBtn)]) {
                        rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(rtkBtn)] = true;
                    } else if (!pressed && rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(rtkBtn)]) {
                        rtkEvent._gamepadButtonPressed[i][static_cast<std::size_t>(rtkBtn)] = false;
                        rtkEvent._gamepadButtonReleased[i][static_cast<std::size_t>(rtkBtn)] = true;
                    }
                };

                checkButton(XINPUT_GAMEPAD_A, GamepadButton::A);
                checkButton(XINPUT_GAMEPAD_B, GamepadButton::B);
                checkButton(XINPUT_GAMEPAD_X, GamepadButton::X);
                checkButton(XINPUT_GAMEPAD_Y, GamepadButton::Y);
                checkButton(XINPUT_GAMEPAD_DPAD_UP, GamepadButton::DpadUp);
                checkButton(XINPUT_GAMEPAD_DPAD_DOWN, GamepadButton::DpadDown);
                checkButton(XINPUT_GAMEPAD_DPAD_LEFT, GamepadButton::DpadLeft);
                checkButton(XINPUT_GAMEPAD_DPAD_RIGHT, GamepadButton::DpadRight);
                checkButton(XINPUT_GAMEPAD_LEFT_SHOULDER, GamepadButton::L1);
                checkButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadButton::R1);
                checkButton(XINPUT_GAMEPAD_LEFT_THUMB, GamepadButton::L3);
                checkButton(XINPUT_GAMEPAD_RIGHT_THUMB, GamepadButton::R3);
                checkButton(XINPUT_GAMEPAD_START, GamepadButton::Start);
                checkButton(XINPUT_GAMEPAD_BACK, GamepadButton::Select);

                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::LeftX)] = 
                    fmaxf(-1.0f, (float)state.Gamepad.sThumbLX / 32767.0f);
                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::LeftY)] = 
                    fmaxf(-1.0f, (float)state.Gamepad.sThumbLY / 32767.0f);
                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::RightX)] = 
                    fmaxf(-1.0f, (float)state.Gamepad.sThumbRX / 32767.0f);
                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::RightY)] = 
                    fmaxf(-1.0f, (float)state.Gamepad.sThumbRY / 32767.0f);
                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::L2)] = 
                    (float)state.Gamepad.bLeftTrigger / 255.0f;
                rtkEvent._gamepadAxis[i][static_cast<std::size_t>(GamepadAxis::R2)] = 
                    (float)state.Gamepad.bRightTrigger / 255.0f;
            } else {
                rtkEvent._gamepadConnected[i] = false;
            }
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                _isOpen = false;
            } else if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) {
                WPARAM key = msg.wParam;
                if (key == VK_SHIFT) {
                    key = MapVirtualKey((msg.lParam & 0x00FF0000) >> 16, MAPVK_VSC_TO_VK_EX);
                } else if (key == VK_CONTROL) {
                    key = (msg.lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
                } else if (key == VK_MENU) {
                    key = (msg.lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
                }
                if (key < RTK_KEYS_TAB_SIZE) {
                    rtkEvent._keyPressed[key] = true;
                }
            } else if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP) {
                WPARAM key = msg.wParam;
                if (key == VK_SHIFT) {
                    key = MapVirtualKey((msg.lParam & 0x00FF0000) >> 16, MAPVK_VSC_TO_VK_EX);
                } else if (key == VK_CONTROL) {
                    key = (msg.lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
                } else if (key == VK_MENU) {
                    key = (msg.lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
                }
                if (key < RTK_KEYS_TAB_SIZE) {
                    rtkEvent._keyPressed[key] = false;
                    rtkEvent._keyReleased[key] = true;
                }
            } else if (msg.message == WM_KILLFOCUS) {
                memset(rtkEvent._keyPressed, 0, RTK_KEYS_TAB_SIZE);
                memset(rtkEvent._mouseButtonPressed, 0, 3);
            } else if (msg.message == WM_MOUSEMOVE) {
                rtkEvent._mouseX = (int)(short)LOWORD(msg.lParam);
                rtkEvent._mouseY = (int)(short)HIWORD(msg.lParam);
            } else if (msg.message == WM_LBUTTONDOWN) {
                rtkEvent._mouseButtonPressed[0] = true;
            } else if (msg.message == WM_LBUTTONUP) {
                rtkEvent._mouseButtonPressed[0] = false;
                rtkEvent._mouseButtonReleased[0] = true;
            } else if (msg.message == WM_RBUTTONDOWN) {
                rtkEvent._mouseButtonPressed[1] = true;
            } else if (msg.message == WM_RBUTTONUP) {
                rtkEvent._mouseButtonPressed[1] = false;
                rtkEvent._mouseButtonReleased[1] = true;
            } else if (msg.message == WM_MBUTTONDOWN) {
                rtkEvent._mouseButtonPressed[2] = true;
            } else if (msg.message == WM_MBUTTONUP) {
                rtkEvent._mouseButtonPressed[2] = false;
                rtkEvent._mouseButtonReleased[2] = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return _isOpen;
    }

    std::vector<const char*> Window::getRequiredExtensions() const
    {
        return { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    }

    void Window::createSurface(void *vkInstance)
    {
        _vkInstance = vkInstance;
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = reinterpret_cast<HWND>(_windowHandle);
        createInfo.hinstance = reinterpret_cast<HINSTANCE>(_display);

        VkSurfaceKHR surface;
        if (vkCreateWin32SurfaceKHR((VkInstance)vkInstance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
            LOG_ERROR("Failed to create Win32 Vulkan surface");
        } else {
            _surface = surface;
            LOG_INFO("Win32 Vulkan surface created");
        }
    }

    VkSurfaceKHR Window::getSurface() const { return _surface; }
}
