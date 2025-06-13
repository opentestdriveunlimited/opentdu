#include "system_win32.h"
#include "tdu_instance.h"
#include "shared.h"
#include "core/assert.h"

#if OTDU_WIN32
SystemImpl* gpSystemImpl = nullptr;

LRESULT Wndproc(
    HWND param_1,
    UINT param_2,
    WPARAM wParam,
    LPARAM lParam
)
{
    if ( gpSystemImpl->KeyboardInputCallback != nullptr ) {
        return gpSystemImpl->KeyboardInputCallback( param_1, param_2, wParam, lParam );
    }

    if ( param_2 < WM_QUERYENDSESSION ) {
        if ( param_2 == WM_CLOSE ) {
            if ( gpSystemImpl->ExitCallback != nullptr ) {
                gpSystemImpl->ExitCallback();
            }
            return 0;
        }

        switch ( param_2 ) {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;

        case WM_SIZE:
            if ( wParam == SIZE_MINIMIZED ) {
                gpSystemImpl->bWindowMinimized = true;
                if ( gpSystemImpl->WindowMinimizedCallback ) {
                    gpSystemImpl->WindowMinimizedCallback( gpSystemImpl->bWindowMinimized );
                }
                gpSystemImpl->bWindowVisible = false;
                return 0;
            } else if ( wParam == SIZE_MAXIMIZED ) {
                gpSystemImpl->bWindowMinimized = false;
                if ( gpSystemImpl->WindowMinimizedCallback ) {
                    gpSystemImpl->WindowMinimizedCallback( gpSystemImpl->bWindowMinimized );
                }
                gpSystemImpl->bWindowVisible = true;
                return 0;
            } else {
                return 0;
            }

            if ( !gpSystemImpl->bWindowVisible ) {
                if ( !gpSystemImpl->bWindowMinimized ) {
                    return 0;
                }
                gpSystemImpl->bWindowMinimized = false;
                if ( gpSystemImpl->WindowMinimizedCallback ) {
                    gpSystemImpl->WindowMinimizedCallback( gpSystemImpl->bWindowMinimized );
                }
                return 0;
            }
            gpSystemImpl->bWindowVisible = false;
            break;

        case WM_SETFOCUS:
            gpSystemImpl->bWindowHasFocus = true;
            if ( gpSystemImpl->WindowFocusCallback ) {
                gpSystemImpl->WindowFocusCallback( gpSystemImpl->bWindowHasFocus );
            }
            return 0;

        case WM_KILLFOCUS:
            gpSystemImpl->bWindowHasFocus = false;
            if ( gpSystemImpl->WindowFocusCallback ) {
                gpSystemImpl->WindowFocusCallback( gpSystemImpl->bWindowHasFocus );
            }
            return 0;

        default:
            return DefWindowProcA( param_1, param_2, wParam, lParam );
        }
    } else {
        if ( param_2 == WM_ACTIVATEAPP ) {
            bool bWindowActivationState = ( wParam == TRUE );
            if ( gpSystemImpl->bWindowActivated == bWindowActivationState ) {
                return 0;
            }
            gpSystemImpl->bWindowActivated = bWindowActivationState;

            if ( gpSystemImpl->WindowActivationCallback ) {
                gpSystemImpl->WindowActivationCallback( gpSystemImpl->bWindowActivated );
            }
            return 0;
        } else if ( param_2 == WM_SYSCOMMAND ) {
            switch ( wParam ) {
            case SC_KEYMENU:
            case SC_TASKLIST:
            case SC_SCREENSAVE:
            case SC_MONITORPOWER:
                return 0;
            default:
                return DefWindowProcA( param_1, param_2, wParam, lParam );
            }
        }

        if ( param_2 != WM_EXITSIZEMOVE ) {
            return DefWindowProcA( param_1, param_2, wParam, lParam );
        }
    }
 
    return 0;
}

static void RequestExit()
{
    if ( gpTestDriveInstance ) {
        gpTestDriveInstance->requestExit();
    }
}

static void SetWindowPosCallback( bool bActivated )
{
    if ( !bActivated ) {
        return;
    }
    DWORD dwStyle = GetWindowLongA( gpSystemImpl->Hwnd, GWL_STYLE );

    int32_t top = 0;
    int32_t left = 0;
    int32_t bottom = gpSystemImpl->Height;
    int32_t right = gpSystemImpl->Width;

    RECT rect = { 0 };
    rect.left = left;
    rect.top = top;
    rect.right = gpSystemImpl->Width;
    rect.bottom = gpSystemImpl->Height;
    AdjustWindowRectEx( &rect, dwStyle, 0, 0 );

    int32_t X = 0;
    int32_t Y = 0;
    int32_t cx = right - left;
    int32_t cy = bottom - top;

    if ( gpSystemImpl->bWindowed ) {
        GetWindowRect( GetDesktopWindow(), &rect );
        X = (right - cx) >> 1;
        Y = (bottom - cy) >> 1;
    }

    SetWindowPos( gpSystemImpl->Hwnd, HWND_TOP, X, Y, cx, cy, SWP_NOZORDER );
}

static void Win32PumpSysEvents( void )
{
    int iVar1;
    tagMSG local_1c;

    iVar1 = PeekMessageA( &local_1c, ( HWND )0x0, 0, 0, 1 );
    while ( iVar1 != 0 ) {
        TranslateMessage( &local_1c );
        DispatchMessageA( &local_1c );
        iVar1 = PeekMessageA( &local_1c, ( HWND )0x0, 0, 0, 1 );
    }
}

SystemImpl::SystemImpl()
    : Hwnd( nullptr )
    , Width( 1280 )
    , Height( 720 )
    , SystemCS()
    , StartTime()
    , CounterFrequency()
    , bWindowActivated( 0 )
    , bWindowMinimized( 0 )
    , bWindowVisible( 0 )
    , bWindowHasFocus( 0 )
    , bWindowed( 0 )
{
    OTDU_ASSERT( gpSystemImpl == nullptr );
    gpSystemImpl = this;
}

void SystemImpl::tick(float deltaTime)
{
    Win32PumpSysEvents();
}

bool SystemImpl::initialize()
{
    bool bRegisteredWinClass = registerClass();
    if ( !bRegisteredWinClass ) {
        return false;
    }

    bool bCreatedMainWindow = createWindow();
    if ( !bCreatedMainWindow ) {
        return false;
    }

    // TODO:
    //HANDLE gameIcon = LoadImageA( gHINSTANCE, ( LPCSTR )0x65, 1, 0, 0, 0x40 );
    //SetClassLongPtr( Hwnd, ( -14 ), (LONG)gameIcon );

    WindowActivationCallback = SetWindowPosCallback;
    ExitCallback = RequestExit;

    InitializeCriticalSection( &SystemCS );

    BOOL BVar2 = QueryPerformanceFrequency( &CounterFrequency );
    BVar2 &= QueryPerformanceFrequency( &StartTime );
    
    if ( BVar2 == 0 ) {
        CounterFrequency.QuadPart = 0;
        StartTime.QuadPart = timeGetTime();
    }
    
    return true;
}

void SystemImpl::terminate()
{
    Win32PumpSysEvents();
    DeleteCriticalSection( &SystemCS );
}

bool SystemImpl::registerClass()
{
    WNDCLASSA wndClass;
    wndClass.lpszClassName = "OpenTDUClient";
    wndClass.lpfnWndProc = Wndproc;
    wndClass.style = 3;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = gHINSTANCE;
    wndClass.hIcon = LoadIconA( ( HINSTANCE )0x0, ( LPCSTR )0x7f00 );
    wndClass.hCursor = LoadCursorA( ( HINSTANCE )0x0, ( LPCSTR )0x7f00 );
    wndClass.hbrBackground = ( HBRUSH )GetStockObject( 4 );
    wndClass.lpszMenuName = ( LPCSTR )0x0;

    return RegisterClassA( &wndClass ) != 0;
}

bool SystemImpl::createWindow()
{
    BOOL BVar2;
    DWORD dwExStyle;
    int X;
    int Y;
    tagRECT local_20;
    tagRECT tStack_10;

    local_20.top = 0;
    local_20.left = 0;
    local_20.right = Width;
    local_20.bottom = Height;
    Hwnd = GetDesktopWindow();
    GetWindowRect( Hwnd, &tStack_10 );
    X = ( tStack_10.right - local_20.right ) / 2;
    Y = ( tStack_10.bottom - local_20.bottom ) / 2;
    BVar2 = AdjustWindowRectEx( &local_20, 0x80cf0000, 0, WS_EX_APPWINDOW );
    if ( BVar2 != 0 ) {
        tStack_10.right = local_20.right - local_20.left;
        tStack_10.bottom = local_20.bottom - local_20.top;
        dwExStyle = WS_EX_APPWINDOW;

        tStack_10.left = X;
        tStack_10.top = Y;
        Hwnd = CreateWindowExA( dwExStyle, "OpenTDUClient", "OpenTDU",
                                 WS_OVERLAPPEDWINDOW, X, Y, tStack_10.right, tStack_10.bottom, ( HWND )0x0, ( HMENU )0x0,
                                  gHINSTANCE, ( LPVOID )0x0 );

        if ( Hwnd != ( HWND )0x0 ) {
            UpdateWindow( Hwnd );
            ShowWindow( Hwnd, gNCmdShow );
            return true;
        }
    }
    return false;
}
#endif