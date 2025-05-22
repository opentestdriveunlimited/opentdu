#include "shared.h"
#include "gs_camera.h"

GSCamera* gpCamera = nullptr;

GSCamera::GSCamera()
    : DeadZone( 0.0f )
    , NearPlane( 0.5f )
    , FarPlane( 40000.0f )
{
    gpCamera = this;
}

GSCamera::~GSCamera()
{

}

bool GSCamera::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCamera::tick()
{

}

void GSCamera::terminate()
{

}
