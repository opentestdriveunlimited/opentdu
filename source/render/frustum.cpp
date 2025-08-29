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
    float fVar8 = 1.0f / fVar9;
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

void Frustum::setCamera(Camera* param_2, bool param_3)
{
    pCamera = param_2;
    bIs2DProjection = param_3;
}

void Frustum::setOrthoDimensions(float width, float height)
{
    orthoWidth = width;
    orthoHeight = height;
    
    orthoProjectionMatrix.col(0)[0] = 3.3333333f;
    orthoProjectionMatrix.col(1)[1] = 2.0f / orthoHeight;
}

int32_t Frustum::testBoundingSphere(Eigen::Vector4f &param_2, float param_3)
{
    // FUN_0050b980
    float fVar2 = 0.0f;
    float fVar3 = 0.0f;
    float fVar4 = 0.0f;
    float fVar5 = 0.0f;
    float fVar6 = 0.0f;
    float fVar7 = 0.0f;

    if (!bIs2DProjection) {
        fVar2 = 0.0f - param_2.z();
        fVar3 = abs(param_2.x()) * pCamera->getInvAspectRatio();
        fVar6 = abs(param_2.y()) * pCamera->getInvAspectRatio();
        fVar4 = abs(param_2.z());
        if ((zLimits.x() <= fVar2 + param_3) && (fVar2 - param_3 <= zLimits.y())) {
            float fVar5 = pCamera->getCosHalfFov() * fVar6 - pCamera->getSinHalfFov() * fVar4;
            fVar2 = pCamera->getTanHalfFov() * fVar4;
            fVar4 = pCamera->getCosHalfFov() * fVar3 - pCamera->getSinHalfFov() * fVar4;
            if ((fVar2 <= fVar3) || (fVar2 <= fVar6)) {
                if (param_3 < fVar4) {
                    return 0;
                }
                if (param_3 < fVar5) {
                    return 0;
                }
            } else if ((param_3 < abs(fVar5)) && (param_3 < abs(fVar4))) {
                return 1;
            }
            return 2;
        }
    } else {
        fVar3 = orthoWidth * 0.5f;
        fVar4 = orthoHeight * 0.5f;
        fVar6 = param_2.x() + param_3;
        fVar2 = param_3 + param_2.y();
        fVar5 = param_2.x() - param_3;
        fVar7 = param_2.y() - param_3;
        if ((((0.0f - fVar3 <= fVar6) && (0.0f - fVar4 <= fVar2)) && (fVar5 <= fVar3)) && (fVar7 <= fVar4)) {
            if (fVar3 < fVar6) {
                return 2;
            }
            if (fVar4 < fVar2) {
                return 2;
            }
            if (fVar5 < 0.0 - fVar3) {
                return 2;
            }
            if (fVar7 < 0.0 - fVar4) {
                return 2;
            }
            return 1;
        }
    }

    return 0;
}
