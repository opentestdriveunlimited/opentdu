#pragma once

#include "camera_base.h"

class CameraGame : public Camera {
public:
    CameraGame();
    ~CameraGame();

    void initialize();
    bool tick(float deltaTime);

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

    Eigen::Vector3f worldPosition;
    Eigen::Vector3f upVector;
    Eigen::Vector3f velocity;
    Eigen::Vector3f worldTarget;

    Eigen::Vector3f worldPositionWithUnknownOffset;
    Eigen::Vector3f worldTargetWithUnknownOffset;

    uint8_t bInitialized : 1;
};
