#pragma once

#if OTDU_WIN32
#include "shared.h"

using KeyboardInputCallback_t = std::function< int32_t( HWND, UINT, WPARAM, LPARAM ) >;
using ExitCallback_t = std::function< void() >;
using WindowFlagCallback_t = std::function< void( bool ) >;

struct SystemImpl {
    SystemImpl();

    void tick(float deltaTime);
    bool initialize();
    void terminate();

    HWND                        Hwnd;

    uint32_t                    Width;
    uint32_t                    Height;

    KeyboardInputCallback_t     KeyboardInputCallback;
    ExitCallback_t              ExitCallback;
    WindowFlagCallback_t        WindowMinimizedCallback;
    WindowFlagCallback_t        WindowFocusCallback;
    WindowFlagCallback_t        WindowActivationCallback;

    CRITICAL_SECTION            SystemCS;

    LARGE_INTEGER               StartTime;
    LARGE_INTEGER               CounterFrequency;

    uint8_t                     bWindowActivated : 1;
    uint8_t                     bWindowMinimized : 1;
    uint8_t                     bWindowVisible : 1;
    uint8_t                     bWindowHasFocus : 1;
    uint8_t                     bWindowed : 1;

private:
    bool registerClass();
    bool createWindow();
};
#endif
