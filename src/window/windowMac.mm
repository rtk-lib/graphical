#ifndef VK_USE_PLATFORM_METAL_EXT
#define VK_USE_PLATFORM_METAL_EXT
#endif
#include "window/window.hpp"
#include "Logger/Logger.hpp"
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/CAMetalLayer.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>

@interface RtkMetalView : NSView <CALayerDelegate>
@end
@implementation RtkMetalView
- (BOOL)acceptsFirstResponder { return YES; }
- (void)keyDown:(NSEvent*)event { (void)event; }
- (void)keyUp:(NSEvent*)event { (void)event; }
@end

@interface RtkWindowDelegate : NSObject <NSWindowDelegate> {
@public
    bool* isOpen;
    bool pressed[RTK_KEYS_TAB_SIZE];
    bool released[RTK_KEYS_TAB_SIZE];
    bool mousePressed[3];
    bool mouseReleased[3];
    int mouseX;
    int mouseY;
}
@property (retain) NSWindow* ownedWindow;
- (void)releaseKeys;
- (void)updateDrawable;
@end

@implementation RtkWindowDelegate
@synthesize ownedWindow = _ownedWindow;
- (void)releaseKeys {
    for (unsigned i = 0; i < RTK_KEYS_TAB_SIZE; ++i) {
        if (pressed[i]) released[i] = true;
        pressed[i] = false;
    }
    for (unsigned i = 0; i < 3; ++i) {
        if (mousePressed[i]) mouseReleased[i] = true;
        mousePressed[i] = false;
    }
}
- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    [self releaseKeys];
    if (isOpen) *isOpen = false;
}
- (void)windowDidResignKey:(NSNotification*)notification {
    (void)notification;
    [self releaseKeys];
}
- (void)updateDrawable {
    NSView* view = self.ownedWindow.contentView;
    CAMetalLayer* layer = (CAMetalLayer*)view.layer;
    NSRect pixels = [view convertRectToBacking:view.bounds];
    layer.contentsScale = self.ownedWindow.backingScaleFactor;
    layer.drawableSize = CGSizeMake(pixels.size.width, pixels.size.height);
}
- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    [self updateDrawable];
}
- (void)windowDidChangeBackingProperties:(NSNotification*)notification {
    (void)notification;
    [self updateDrawable];
}
#if !__has_feature(objc_arc)
- (void)dealloc {
    [_ownedWindow release];
    [super dealloc];
}
#endif
@end

namespace {
RtkWindowDelegate* getState(void* pointer) {
#if __has_feature(objc_arc)
    return (__bridge RtkWindowDelegate*)pointer;
#else
    return (RtkWindowDelegate*)pointer;
#endif
}
}

namespace rtk {
    Window::Window(uint32_t width, uint32_t height, const char* title)
        : _display(nullptr), _windowHandle(0), _vkInstance(nullptr),
          _surface(VK_NULL_HANDLE), _isOpen(false), _width(width), _height(height)
    {
        if (![NSThread isMainThread])
            throw std::runtime_error("Create Cocoa Window on the main thread");
        if (!width || !height)
            throw std::runtime_error("Window dimensions must be nonzero");
        @autoreleasepool {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp finishLaunching];
            NSWindow* window = [[NSWindow alloc]
                initWithContentRect:NSMakeRect(0, 0, width, height)
                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                           NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
                backing:NSBackingStoreBuffered defer:NO];
            if (!window) throw std::runtime_error("Failed to create Cocoa window");
            window.releasedWhenClosed = NO;
            window.acceptsMouseMovedEvents = YES;
            NSString* text = title ? [NSString stringWithUTF8String:title] : nil;
            window.title = text ? text : @"rtk-lib";
            RtkMetalView* view = [[RtkMetalView alloc]
                initWithFrame:NSMakeRect(0, 0, width, height)];
            view.wantsLayer = YES;
            CAMetalLayer* layer = [CAMetalLayer layer];
            layer.delegate = view;
            view.layer = layer;
            window.contentView = view;
            [window makeFirstResponder:view];
            RtkWindowDelegate* state = [[RtkWindowDelegate alloc] init];
            state.ownedWindow = window;
            state->isOpen = &_isOpen;
            window.delegate = state;
    #if __has_feature(objc_arc)
            _display = (__bridge_retained void*)state;
            _windowHandle = reinterpret_cast<uintptr_t>((__bridge void*)window);
    #else
            _display = state;
            _windowHandle = reinterpret_cast<uintptr_t>((void*)window);
            [view release];
            [window release];
    #endif
            _isOpen = true;
            [state updateDrawable];
            [window makeKeyAndOrderFront:nil];
            [NSApp activateIgnoringOtherApps:YES];
            LOG_INFO("Mac Cocoa window created");
        }
    }

    Window::~Window() {
        @autoreleasepool {
            if (_display) {
                RtkWindowDelegate* state = getState(_display);
                state->isOpen = nullptr;
                state.ownedWindow.delegate = nil;
                [state.ownedWindow close];
    #if __has_feature(objc_arc)
                RtkWindowDelegate* owner =
                    (__bridge_transfer RtkWindowDelegate*)_display;
                (void)owner;
    #else
                [state release];
    #endif
            }
            _display = nullptr;
            _windowHandle = 0;
            _isOpen = false;
        }
    }

    void Window::display(RGB clearColor) {
        (void)clearColor;
    }
    VkSurfaceKHR Window::getSurface() const { return _surface; }

    std::vector<const char*> Window::getRequiredExtensions() const {
        return {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME};
    }

    void Window::createSurface(void* vkInstance) {
        if (![NSThread isMainThread])
            throw std::runtime_error("Create Cocoa surface on the main thread");
        if (!vkInstance || !_display || !_isOpen)
            throw std::runtime_error("Cannot create surface: invalid instance/window");
        if (_surface != VK_NULL_HANDLE)
            throw std::runtime_error("Window already has a Vulkan surface");
        VkInstance instance = static_cast<VkInstance>(vkInstance);
        auto createMetalSurface = reinterpret_cast<PFN_vkCreateMetalSurfaceEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT"));
        if (!createMetalSurface)
            throw std::runtime_error("VK_EXT_metal_surface is not enabled/available");
        @autoreleasepool {
            RtkWindowDelegate* state = getState(_display);
            [state updateDrawable];
            VkMetalSurfaceCreateInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            info.pLayer = (CAMetalLayer*)state.ownedWindow.contentView.layer;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            VkResult result = createMetalSurface(instance, &info, nullptr, &surface);
            if (result != VK_SUCCESS)
                throw std::runtime_error("Failed to create Metal surface: " +
                                         std::to_string(static_cast<int>(result)));
            _surface = surface;
            _vkInstance = vkInstance;
        }
    }

    bool Window::pollEvents(rtk::Event& rtkEvent) {
        if (![NSThread isMainThread])
            throw std::runtime_error("Poll Cocoa events on the main thread");
        std::fill(std::begin(rtkEvent._keyPressed), std::end(rtkEvent._keyPressed), false);
        std::fill(std::begin(rtkEvent._keyReleased), std::end(rtkEvent._keyReleased), false);
        std::fill(std::begin(rtkEvent._mouseButtonPressed), std::end(rtkEvent._mouseButtonPressed), false);
        std::fill(std::begin(rtkEvent._mouseButtonReleased), std::end(rtkEvent._mouseButtonReleased), false);
        if (!_display) return false;
        @autoreleasepool {
            RtkWindowDelegate* state = getState(_display);
            NSView* view = state.ownedWindow.contentView;
            NSRect pixels = [view convertRectToBacking:view.bounds];
            setWindowSize(pixels.size.width, pixels.size.height);
            NSEvent* event = nil;
            while (_isOpen && (event = [NSApp nextEventMatchingMask:NSEventMaskAny
                    untilDate:[NSDate distantPast]
                    inMode:NSDefaultRunLoopMode dequeue:YES])) {
                if (event.window == state.ownedWindow) {
                    const NSEventType type = event.type;
                    if (type == NSEventTypeKeyDown || type == NSEventTypeKeyUp ||
                        type == NSEventTypeFlagsChanged) {
                        const unsigned code = event.keyCode;
                        if (code < RTK_KEYS_TAB_SIZE) {
                            bool down = type == NSEventTypeKeyDown;
                            if (type == NSEventTypeFlagsChanged)
                                down = CGEventSourceKeyState(
                                    kCGEventSourceStateCombinedSessionState,
                                    static_cast<CGKeyCode>(code));
                            if (!down) state->released[code] = true;
                            state->pressed[code] = down;
                        }
                    } else if (type == NSEventTypeLeftMouseDown || type == NSEventTypeLeftMouseUp ||
                               type == NSEventTypeRightMouseDown || type == NSEventTypeRightMouseUp ||
                               type == NSEventTypeOtherMouseDown || type == NSEventTypeOtherMouseUp) {
                        int button = -1;
                        if (type == NSEventTypeLeftMouseDown || type == NSEventTypeLeftMouseUp) button = 0;
                        else if (type == NSEventTypeRightMouseDown || type == NSEventTypeRightMouseUp) button = 1;
                        else if (type == NSEventTypeOtherMouseDown || type == NSEventTypeOtherMouseUp) {
                            if (event.buttonNumber == 2) button = 2;
                        }

                        if (button != -1) {
                            bool down = (type == NSEventTypeLeftMouseDown || type == NSEventTypeRightMouseDown || type == NSEventTypeOtherMouseDown);
                            if (!down) state->mouseReleased[button] = true;
                            state->mousePressed[button] = down;
                        }
                    }
                    if (type == NSEventTypeMouseMoved || type == NSEventTypeLeftMouseDragged ||
                        type == NSEventTypeRightMouseDragged || type == NSEventTypeOtherMouseDragged ||
                        type == NSEventTypeLeftMouseDown || type == NSEventTypeRightMouseDown ||
                        type == NSEventTypeOtherMouseDown || type == NSEventTypeLeftMouseUp ||
                        type == NSEventTypeRightMouseUp || type == NSEventTypeOtherMouseUp) {
                        NSPoint point = [event locationInWindow];
                        NSRect contentRect = [state.ownedWindow.contentView frame];
                        state->mouseX = (int)point.x;
                        state->mouseY = (int)(contentRect.size.height - point.y);
                    }
                }
                [NSApp sendEvent:event];
            }
            [NSApp updateWindows];
            if (!state.ownedWindow.isKeyWindow || !_isOpen) [state releaseKeys];
            std::copy(std::begin(state->pressed), std::end(state->pressed),
                      std::begin(rtkEvent._keyPressed));
            std::copy(std::begin(state->released), std::end(state->released),
                      std::begin(rtkEvent._keyReleased));
            std::fill(std::begin(state->released), std::end(state->released), false);
            std::copy(std::begin(state->mousePressed), std::end(state->mousePressed),
                      std::begin(rtkEvent._mouseButtonPressed));
            std::copy(std::begin(state->mouseReleased), std::end(state->mouseReleased),
                      std::begin(rtkEvent._mouseButtonReleased));
            std::fill(std::begin(state->mouseReleased), std::end(state->mouseReleased), false);
            rtkEvent._mouseX = state->mouseX;
            rtkEvent._mouseY = state->mouseY;
        }
        return _isOpen;
    }
} // namespace rtk