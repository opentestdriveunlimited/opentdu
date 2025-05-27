#pragma once

#include "core/notifier.h"
#include "core/color.h"
#include "paint.h"

struct CarConfig
{
    uint32_t Hash;
    int32_t PowerCC;
    int32_t Weight;
    std::string FullName;
    std::string Name;
    std::string Model;
    std::string SubModel;
    std::string ConfigName;
    std::string BankGeometryName;
    std::string BankPhysicsName;
    float MaxSpeed;
    float TimeTo100Kmh;
    float TimeTo60Mph;

    CarConfig()
        : Hash( 0 )
        , PowerCC( 0 )
        , Weight( 0 )
        , FullName("")
        , Name("")
        , Model("")
        , SubModel("")
        , ConfigName("")
        , BankGeometryName("")
        , BankPhysicsName("")
        , MaxSpeed(0.0f)
        , TimeTo100Kmh(0.0f)
        , TimeTo60Mph(0.0f)
    {
        
    }
};

class CarDesc : public Notifiable
{
public:
    enum class eState {
        CDS_None = 0,
        CDS_PutOnEbay = 1,
        CDS_Invalid = 2,
        CDS_Corrupted = 4,
        CDS_Rental = 5,
        CDS_TestDrive = 6,
        CDS_SoldOnEbay = 7
    };

public:
    inline uint32_t getCarID() const { return carID; }

public:
    CarDesc();
    ~CarDesc();

private:
    uint32_t carConfigID;
    uint32_t carID;
    eState state;
    CarConfig* pConfig;
    float upgradeLevel;
    ePaintType paintType;
    int32_t paintIndex;
    ColorRGBA paintColor;
    ColorRGBA paintLogo;
    ColorRGBA paintSecondColor;
    ColorRGBA paintSecondColor2;
    int32_t rimID;
    int32_t interiorID;
    int32_t price;
    float odometer;
    float usageTime;
    uint64_t packHashcode;
};
