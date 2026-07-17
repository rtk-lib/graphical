/*
 * Mac (Cocoa) Window implementation
 */
#include "window.hpp"
#include "../Logger/Logger.hpp"
#import <Cocoa/Cocoa.h>

@interface RtkWindowDelegate : NSObject <NSWindowDelegate>
@property (assign) bool* isOpen;
@end

@implementation RtkWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    if (_isOpen) *_isOpen = false;
}
@end

namespace rtk 
{
    Window::Window(uint32_t width, uint32_t height, const char* title) : _isOpen(true)
    {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        NSRect frame = NSMakeRect(0, 0, width, height);
        NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                             styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                                             backing:NSBackingStoreBuffered
                                             defer:NO];
        
        [window setTitle:[NSString stringWithUTF8String:title]];
        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
        
        RtkWindowDelegate* delegate = [[RtkWindowDelegate alloc] init];
        delegate.isOpen = &_isOpen;
        [window setDelegate:delegate];
        
        _display = (__bridge_retained void*)delegate;
        _windowHandle = (uint64_t)(__bridge void*)window;
        _surface = 0;
        _vkInstance = nullptr;

        LOG_INFO("Mac Cocoa window created");
    }

    Window::~Window()
    {
        if (_windowHandle) {
            NSWindow* window = (__bridge NSWindow*)(void*)_windowHandle;
            [window close];
            RtkWindowDelegate* delegate = (__bridge_transfer RtkWindowDelegate*)_display;
            delegate = nil;
            LOG_INFO("Mac Cocoa window destroyed");
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
        
        NSEvent* event;
        do {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES];
            if (event) {
                [NSApp sendEvent:event];
            }
        } while (event != nil);
        
        return _isOpen;
    }
}
