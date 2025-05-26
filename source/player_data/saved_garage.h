#pragma once

#include "core/notifier.h"
#include "game/vehicle/car_desc.h"

struct SavedGarageSlot
{
    CarDesc  Car;
    uint64_t HouseHashcode;
    uint32_t EbayCarIndex;
};

class SavedGarage : public Notifiable
{
public:
    SavedGarage();
    ~SavedGarage();

private:
    std::vector<SavedGarageSlot> slots;
    uint32_t currentCarIndex;
};
