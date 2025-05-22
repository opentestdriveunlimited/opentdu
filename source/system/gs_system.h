#pragma once

#include "game/gs_base.h"
#include "config/gs_config.h"

//#if OTDU_WIN32
//#include "system_win32.h"
//#endif

class GSSystem : public GameSystem {
public:
    const char* getName() const override { return "Service : Sys"; }

public:
    GSSystem();
    ~GSSystem();

    bool initialize( TestDriveGameInstance* ) override;

    void tick() override;
    void terminate() override;

    void resizeGameWindow( const uint32_t width, const uint32_t height, const eFullscreenMode fullscreenMode, const bool bWindowed );

private:
    struct GLFWwindow* pWindow;
    
private:
    void centerWindow();
};

extern GSSystem* gpSystem;
