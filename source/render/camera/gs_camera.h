#pragma once

#include "game/gs_base.h"

class Camera;

class GSCamera : public GameSystem {
public:

public:
    const char* getName() const override { return "Camera"; }
    inline void setDeadZone( const float value ) { deadZone = value; }

    inline void setNearPlane( const float value ) { nearPlane = value; }
    inline void setFarPlane( const float value ) { farPlane = value; }
    inline void setInteriorNearPlane( const float value ) { nearPlaneInterior = value; }
    inline void setInteriorFarPlane( const float value ) { farPlaneInterior= value; }

    inline void setEnableDepthAware( const bool value ) { bDepthAware = value; }
    inline void setEnableDepthAwareInterior( const bool value ) { bDepthAwareInterior = value; }

public:
    GSCamera();
    ~GSCamera();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

private:
    float nearPlane;
    float farPlane;

    float nearPlaneInterior;
    float farPlaneInterior;
    
    float deadZone;

    uint8_t bDepthAware : 1;
    uint8_t bDepthAwareInterior : 1;

    std::vector<Camera*> cameraRegister;
};

extern GSCamera* gpCamera;