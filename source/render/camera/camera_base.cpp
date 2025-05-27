#include "shared.h"
#include "camera_base.h"

CameraBase::CameraBase()
    : fov( 0.7853982f )
    , aspectRatio( 1.3333334f )
    , invAspectRatio( 0.75f )
    , cosHalfFov( cos(0.39269909262657166f) )
    , sinHalfFov( sin(0.39269909262657166f) )
    , tanHalfFov( sinHalfFov / cosHalfFov )
    , focalInfos{ 0.0f, 0.0f, 1000.0f }
    , worldToCam( Eigen::Matrix4f::Identity() )
    , invWorldToCam( Eigen::Matrix4f::Identity() )
{

}

CameraBase::~CameraBase()
{
    
}

void CameraBase::updateMatrix(const Eigen::Vector4f &position, const Eigen::Vector4f &upVector, const Eigen::Vector4f &lookAt)
{
    Eigen::Vector4f viewDirection = lookAt - position;
    computeMatrices(position, upVector, viewDirection);
}

void CameraBase::computeMatrices(const Eigen::Vector4f &position, const Eigen::Vector4f &upVector, const Eigen::Vector4f &viewVector)
{
    
}
