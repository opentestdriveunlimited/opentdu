#pragma once

#include "game/gm_base.h"
#include "core/color.h"
#include "flash/mng_flash.h"

#include "flash_list_car_showcase.h"
#include "mng_flash_car_showcase.h"
#include "render/mng_number.h"

class FlashPlayer;
struct CarState;

class GMCarShowcase : public GameMode {
public:
    GMCarShowcase();
    ~GMCarShowcase();

    virtual void initialize() override;
    virtual void reset() override;

    bool onMessage(FlashMessage& pMessage, FlashPlayer* pPlayer);

private:
    static constexpr FlashMessage kPreviousCar = FlashMessage(0x70726361u);
    static constexpr FlashMessage kNextCar = FlashMessage(0x6e656361u);
    static constexpr FlashMessage kChooseCar = FlashMessage(0x63616368u);
    static constexpr FlashMessage kExit = FlashMessage(0x71756974u);
    
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

    CarState*   pVehicleState;

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
