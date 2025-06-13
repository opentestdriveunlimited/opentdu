#pragma once

#include "saved_profile.h"
#include "saved_car_reserve.h"

class Savegame
{
public:
    Savegame();
    ~Savegame();

private:
    uint32_t        saveIndex;
    SavedProfile    profile;
    SavedCarReserve carReservations;
    
};
