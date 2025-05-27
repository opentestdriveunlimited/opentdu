#pragma once

#include "camera_base.h"

class CameraGame : public CameraBase {
public:
    CameraGame();
    ~CameraGame();

    void initialize();
    bool tick();

protected:
    std::string name;
    int32_t  cameraID;
    uint32_t carID;
    int32_t  cameraType;
    uint32_t flags;

    float nearPlane;
    float farPlane;
    float interiorNearPlane;
    float interiorFarPlane;

    Eigen::Vector4f worldPosition;
    Eigen::Vector4f upVector;
    Eigen::Vector4f velocity;
    Eigen::Vector4f worldTarget;

    Eigen::Vector4f worldPositionWithUnknownOffset;
    Eigen::Vector4f worldTargetWithUnknownOffset;

    uint8_t bInitialized : 1;
};
