#pragma once

#include "game/gm_base.h"

#include "core/color.h"

class GMCarShowcase : public GameMode {
public:
    GMCarShowcase();
    ~GMCarShowcase();

private:
    DrawList*   pDrawList;
    float       RotationAngleY;
    float       WheelRotationSpeed;
    float       FloorHeight;
    ColorRGBA   BackgroundColor;
    ColorRGBA   ClearColor;
    uint32_t    MaxNumCar;
    uint32_t    CurrentCar;
    float       DoorAngle;
    float       DoorSpeed;
    float       DoorMinAngle;
    uint32_t    NumLights;

    uint32_t    CurrentCarID;
    uint32_t    CurrentCarColor;
    uint32_t    CurrentCarInterior;
    uint32_t    CurrentCarRims;
    
    Eigen::Vector4f Ambient;
    Eigen::Vector4f Diffuse;
    Eigen::Vector4f Specular;
    
    Eigen::Vector4f StreamOrigin;

    uint8_t     bUsePhysicsInput : 1;
    uint8_t     bFreezeCarSwitch : 1;
};
