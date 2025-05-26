#pragma once

#include "game/gm_base.h"
#include "core/color.h"
#include "flash/mng_flash.h"

#include "flash_list_car_showcase.h"
#include "mng_flash_car_showcase.h"

class FlashPlayer;

class GMCarShowcase : public GameMode {
public:
    GMCarShowcase();
    ~GMCarShowcase();

    bool onMessage(FlashMessage& pMessage, FlashPlayer* pPlayer);

private:
    static constexpr const FlashMessage kPreviousCar(0x70726361);
    static constexpr const FlashMessage kNextCar(0x6e656361);
    static constexpr const FlashMessage kChooseCar(0x63616368);
    static constexpr const FlashMessage kExit(0x71756974);
    
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

    FlashListCarShowcase flashList;
    MngFlashCarShowcase  mngFlash;

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
