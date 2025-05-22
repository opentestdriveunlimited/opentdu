#pragma once

struct CarUpgrade
{
    uint64_t PackHashcode;
    uint64_t CarGroup;
    uint64_t CarHashcode;
    int32_t Price;
    float Acceleration;
    float Braking;
    float Handling;
};

struct DBEntryCarPack
{
    uint64_t CarHashcode;
    uint64_t BrandHashcode;
    uint64_t CarBankName;
    uint64_t CarName;
    CarUpgrade Upgrades[3];
};
