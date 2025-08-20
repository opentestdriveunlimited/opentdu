#include "shared.h"
#include "gs_camera.h"

#include "camera_base.h"

GSCamera* gpCamera = nullptr;

GSCamera::GSCamera()
    : GameSystem()
    , deadZone( 0.0f )
    , nearPlane( 0.5f )
    , farPlane( 40000.0f )
{
    cameraRegister.reserve( 16 );

    gpCamera = this;
}

GSCamera::~GSCamera()
{

}

bool GSCamera::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCamera::tick(float totalTime, float deltaTime)
{

}

void GSCamera::terminate()
{

}
