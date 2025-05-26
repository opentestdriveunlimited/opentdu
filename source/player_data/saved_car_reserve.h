#pragma once

#include "core/random/bcpl.h"

class SavedCarReserve
{
public:
    SavedCarReserve();
    ~SavedCarReserve();

private:
    struct Reservation {
        uint32_t CarID;
        uint32_t ColorID;
        uint32_t InteriorColorID;
        uint32_t RimsID;
    };

private:
    std::vector<Reservation> carReservations;

    RandomGen randomGenerator;
    uint32_t timeSinceRestock;
    uint32_t lastRestock;
    uint32_t unknown;
    uint32_t seedUpdate;
    uint32_t seedRestock;
};
