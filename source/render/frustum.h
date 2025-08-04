#pragma once

#include "render/camera/camera_base.h"

class Frustum {
public:
    Frustum();
    ~Frustum();

    void updateProjectionMatrices();
    void setPlanes(float nearPlane, float farPlane);

private:
    Eigen::Vector4f zLimits;
    Eigen::Vector4f zProjection;
    Eigen::Matrix4f projectionMatrix;
    Eigen::Matrix4f projectionWMatrix;
    Eigen::Matrix4f orthoProjectionMatrix;

    Camera* pCamera;
    float wProjection;

    float orthoWidth;
    float orthoHeight;
    float offsetX;
    float offsetY;

    uint8_t bIs2DProjection : 1;
};
