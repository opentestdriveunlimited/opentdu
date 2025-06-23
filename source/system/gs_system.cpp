#include "shared.h"
#include "gs_system.h"

#include "tdu_instance.h"
#include "config/gs_config.h"

//#if OTDU_WIN32
//#include "system_win32.h"
//#endif

#ifdef OTDU_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

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

#if OTDU_VULKAN
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
#elif OTDU_OPENGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
#elif OTDU_OPENGL
    glfwMakeContextCurrent( pWindow );
#endif

    return true;
}

void GSSystem::tick(float deltaTime)
{
    glfwPollEvents();
    //glfwSwapBuffers( pWindow );
    //systemImpl.tick(float deltaTime);
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
