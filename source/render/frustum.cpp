#include "shared.h"
#include "frustum.h"

Frustum::Frustum()
    : zLimits(1.0f, 5000.0f, 0.0f, 0.0f)
    , zProjection(1.0f, 5000.0f, 0.0f, 0.0f)
    , projectionMatrix(Eigen::Matrix4f::Identity())
    , projectionWMatrix(Eigen::Matrix4f::Identity())
    , orthoProjectionMatrix(Eigen::Matrix4f::Identity())
    , pCamera( nullptr )
    , wProjection( 0.0f )
    , orthoWidth( 1.0f )
    , orthoHeight( 1.0f )
    , offsetX( 0.0f )
    , offsetY( 0.0f )
    , bIs2DProjection( false )
{
    updateProjectionMatrices();
}

Frustum::~Frustum()
{
}

void Frustum::updateProjectionMatrices()
{
    projectionMatrix = Eigen::Matrix4f::Identity();
    projectionMatrix.row(2)[3] = -1.0f;
    projectionMatrix.row(3)[3] = 0.0f;
    
    orthoProjectionMatrix = Eigen::Matrix4f::Identity();
    orthoProjectionMatrix.row(0)[0] = 2.0f / orthoWidth;
    orthoProjectionMatrix.row(1)[1] = 2.0f / orthoHeight;

    float fVar1 = zProjection.x();
    float fVar7 = zProjection.y();
    float fVar9 = fVar1 - fVar7;
    float fVar8 = 1.0 / fVar9;
    fVar7 = fVar8 * fVar7;

    projectionMatrix.row(2)[2] = fVar7;
    projectionMatrix.row(3)[2] = fVar7 * fVar1;

    orthoProjectionMatrix.row(2)[2] = fVar8;
    orthoProjectionMatrix.row(3)[2] = (-1.0f / fVar9) * fVar1;

    if (pCamera != nullptr) {
        float fVar6 = tan(pCamera->getFOV() * 0.5f);

        projectionMatrix.row(0)[0] = (1.0f / fVar6) / pCamera->getAspectRatio();
        projectionMatrix.row(1)[1] = (1.0f / fVar6);
    }
    
    // Eigen::Matrix4f local_50 = projectionMatrix * Eigen::Vector4f( 1.0f, 0.0f, 0.0f, 0.0f );
    // projectionMatrix = local_50;
}

void Frustum::setPlanes(float nearPlane, float farPlane)
{
    zLimits.x() = nearPlane;
    zProjection.x() = nearPlane;

    float fVar1 = zProjection.y();
    float fVar4 = 1.0f / (nearPlane - fVar1);

    projectionMatrix.col(2)[2] = fVar4 * fVar1;
    projectionMatrix.col(3)[2] = fVar4 * nearPlane * fVar1;

    orthoProjectionMatrix.col(2)[2] = fVar4;
    orthoProjectionMatrix.col(3)[2] = (-1.0f / (nearPlane - fVar1)) * nearPlane;

    zLimits.y() = farPlane;
    zProjection.y() = farPlane;

    fVar1 = zProjection.x();
    float fVar5 = 1.0f / (fVar1 - farPlane);
    fVar4 = fVar5 * farPlane;

    projectionMatrix.col(2)[2] = fVar4;
    projectionMatrix.col(3)[2] = fVar4 * fVar1;

    orthoProjectionMatrix.col(2)[2] = fVar5;
    orthoProjectionMatrix.col(3)[2] = (-1.0f / (fVar1 - farPlane)) * fVar1;

    updateProjectionMatrices();
}