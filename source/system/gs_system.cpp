#include "shared.h"
#include "gs_system.h"

#include "tdu_instance.h"
#include "config/gs_config.h"

//#if OTDU_WIN32
//#include "system_win32.h"
//#endif

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

GSSystem* gpSystem = nullptr;

GSSystem::GSSystem()
    : pWindow( nullptr )
{
    OTDU_ASSERT( gpSystem == nullptr );
    gpSystem = this;
}

GSSystem::~GSSystem()
{

}

bool GSSystem::initialize( TestDriveGameInstance* )
{
    int32_t bGfwlInit = glfwInit();
    if ( bGfwlInit == GLFW_FALSE ) {
        OTDU_LOG_ERROR( "Failed to initialize GFWL (error code: %i)!\n", glfwGetError( NULL ) );
        return false;
    }

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    pWindow = glfwCreateWindow( 1280, 720, "OpenTDU", nullptr, nullptr );
    OTDU_ASSERT( pWindow );

    centerWindow();

    glfwSetWindowCloseCallback( pWindow, []( GLFWwindow* pWindow ) { gpTestDriveInstance->requestExit(); } );
    
#if OTDU_VULKAN
    if ( !glfwVulkanSupported() )
    {
        OTDU_LOG_ERROR( "Fatal: System does not support Vulkan!\n" );
        return false;
    }
#endif

    return true;
}

void GSSystem::tick()
{
    glfwPollEvents();
    //glfwSwapBuffers( pWindow );
    //systemImpl.tick();
}

void GSSystem::terminate()
{
    glfwTerminate();
}

void GSSystem::resizeGameWindow( const uint32_t width, const uint32_t height, const eFullscreenMode fullscreenMode, const bool bWindowed )
{
    if ( bWindowed ) {
        glfwSetWindowSize( pWindow, width, height );
        centerWindow();
    } else {
        const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
        switch ( fullscreenMode ) {
        case eFullscreenMode::FM_Manual:
            glfwSetWindowMonitor( pWindow, glfwGetPrimaryMonitor(), 0, 0, width, height, mode->refreshRate );
            break;
        case eFullscreenMode::FM_Auto:
        case eFullscreenMode::FM_Desktop:
            glfwSetWindowMonitor( pWindow, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate );
            break;
        default:
            OTDU_UNIMPLEMENTED;
            break;
        };
    }
}

void GSSystem::centerWindow()
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode( monitor );

    // Calculate window position
    int windowWidth, windowHeight;
    glfwGetWindowSize( pWindow, &windowWidth, &windowHeight );
    int xpos = ( mode->width - windowWidth ) / 2;
    int ypos = ( mode->height - windowHeight ) / 2;

    // Set window position
    glfwSetWindowPos( pWindow, xpos, ypos );
}
