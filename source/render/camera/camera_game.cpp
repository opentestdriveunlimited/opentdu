#include "shared.h"
#include "camera_game.h"

#include "render/gs_render.h"
#include "gs_camera.h"

CameraGame::CameraGame()
    : Camera()
    , name("")
    , cameraID( 0x186ad )
    , carID( 0x270d )
    , cameraType( 0xffffffff )
    , flags( 0x2001d )
    , nearPlane( 0.8f )
    , farPlane( 10.0f )
    , interiorNearPlane( 1.0f )
    , interiorFarPlane( 10.0f )
    , worldPosition( 0.0f, 0.0f, 0.0f )
    , upVector( kWorldUpVector )
    , velocity( 0.0f, 0.0f, 0.0f )
    , worldTarget( 0.0f, 0.0f, 0.0f )
    , worldPositionWithUnknownOffset( 0.0f, 0.0f, 0.0f )
    , worldTargetWithUnknownOffset( 0.0f, 0.0f, 0.0f )
    , bInitialized( false )
{

}

CameraGame::~CameraGame()
{
    
}

void CameraGame::initialize()
{
    nearPlane = gpRender->getDefaultNearPlane();
    farPlane = gpRender->getDefaultFarPlane();
    
    interiorNearPlane = gpRender->getDefaultInteriorNearPlane();
    interiorFarPlane = gpRender->getDefaultInteriorFarPlane();

    bInitialized = true;
}

bool CameraGame::tick(float deltaTime)
{
    gpCamera->setFarPlane(farPlane);
    gpCamera->setNearPlane(nearPlane);
    gpCamera->setInteriorFarPlane(interiorFarPlane);
    gpCamera->setInteriorNearPlane(interiorNearPlane);
    gpCamera->setEnableDepthAware(true);
    gpCamera->setEnableDepthAwareInterior(true);

    updateMatrix(worldPosition, upVector, worldTarget);

    return true;
}
