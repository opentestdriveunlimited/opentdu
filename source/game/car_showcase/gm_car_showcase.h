#pragma once

#include "game/gm_base.h"
#include "core/color.h"
#include "flash/mng_flash.h"

#include "flash_list_car_showcase.h"
#include "mng_flash_car_showcase.h"
#include "render/mng_number.h"

class FlashPlayer;

class GMCarShowcase : public GameMode {
public:
    GMCarShowcase();
    ~GMCarShowcase();

    void initialize();
    bool onMessage(FlashMessage& pMessage, FlashPlayer* pPlayer);

private:
    static constexpr const FlashMessage kPreviousCar(0x70726361);
    static constexpr const FlashMessage kNextCar(0x6e656361);
    static constexpr const FlashMessage kChooseCar(0x63616368);
    static constexpr const FlashMessage kExit(0x71756974);
    
private:
    DrawList*   pDrawList;
    float       rotationAngleY;
    float       wheelRotationSpeed;
    float       floorHeight;
    ColorRGBA   backgroundColor;
    ColorRGBA   clearColor;
    uint32_t    currentCarHashcode;
    float       doorAngle;
    float       doorSpeed;
    float       doorMinAngle;
    uint32_t    numLights;

    std::vector<uint32_t> carHashes;
    FlashListCarShowcase flashList;
    MngFlashCarShowcase  mngFlash;
    MngNumber mngNumber;

    uint32_t    currentCarID;
    uint32_t    currentCarColor;
    uint32_t    currentCarInterior;
    uint32_t    currentCarRims;
    
    Eigen::Vector4f ambient;
    Eigen::Vector4f diffuse;
    Eigen::Vector4f specular;
    
    Eigen::Vector4f streamOrigin;

    uint8_t     bUsePhysicsInput : 1;
    uint8_t     bFreezeCarSwitch : 1;
};
