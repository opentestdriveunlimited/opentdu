#pragma once

#include "game/gs_base.h"
#include "config/gs_config.h"

#if OTDU_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#endif

struct GLFWwindow;

class GSSystem : public GameSystem {
public:
    const char* getName() const override { return "Service : Sys"; }
    inline const bool isWindowActivated() const { return bWindowActivated; }
    inline GLFWwindow* getWindow() const { return pWindow; }
    
#if OTDU_WIN32
    inline HWND getWindowHandle() const { return glfwGetWin32Window( pWindow ); }
#endif

public:
    GSSystem();
    ~GSSystem();

    bool initialize( TestDriveGameInstance* ) override;

    void tick(float totalTime, float deltaTime) override;
    void terminate() override;

    void resizeGameWindow( const uint32_t width, const uint32_t height, const eFullscreenMode fullscreenMode, const bool bWindowed );

private:
    struct GLFWwindow* pWindow;
    
    uint8_t bWindowActivated : 1;

private:
    void centerWindow();
};

extern GSSystem* gpSystem;
