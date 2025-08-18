#include "shared.h"
#include "camera_base.h"

Camera::Camera()
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

Camera::~Camera()
{
    
}

void Camera::updateMatrix(const Eigen::Vector3f &position, const Eigen::Vector3f &upVector, const Eigen::Vector3f &lookAt)
{
    Eigen::Vector3f viewDirection = lookAt - position;
    computeMatrices(position, upVector, viewDirection);
}

void Camera::setAspectRatio(const float width, const float height)
{
    aspectRatio = width / height;
    invAspectRatio = 1.0f / aspectRatio;
}

void Camera::setDefaultMatrix2D()
{
    // FUN_00404870
    Eigen::Vector3f defaultPosition(0.5f,0.5f,-2.0f);
    Eigen::Vector3f defaultUp(0.0f,-1.0f,0.0f);
    Eigen::Vector3f defaultLookAt(0.5f,0.5f,0.0f);
    
    updateMatrix(defaultPosition, defaultUp, defaultLookAt);
}

void Camera::setFOV(const float param_1)
{
    fov = param_1;
    
    float fVar21 = cos(fov * 0.5f);
    cosHalfFov = fVar21;

    float fVar22 = sin(fov * 0.5f);
    sinHalfFov = fVar22;
    tanHalfFov = fVar22 / fVar21;
}

void Camera::computeMatrices(const Eigen::Vector3f &position, const Eigen::Vector3f &upVector, const Eigen::Vector3f &viewVector)
{
    Eigen::Vector3f local_30 = viewVector * -1.0f;
    local_30.normalize();

    Eigen::Vector3f local_20 = upVector.cross(local_30);
    Eigen::Vector3f local_40 = local_30;
    Eigen::Vector3f pTVar1 = local_40.normalized();

    local_20 = local_30.cross(local_40);
    Eigen::Vector3f local_50 = pTVar1.normalized();
    worldToCam <<   local_40.x(), local_40.y(), local_40.z(), 0.0f,
                    local_50.x(), local_50.y(), local_50.z(), 0.0f,
                    local_30.x(), local_30.y(), local_30.z(), 0.0f,
                    position.x(), position.y(), position.z(), 1.0f;
}
