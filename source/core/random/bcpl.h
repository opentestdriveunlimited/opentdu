#pragma once

#include "shared.h"

// BCPL linear congruential generator used by TDU1
class RandomGen {
public:
    uint32_t getRandomUint32()
    {
         state = state * 0x7ff8a3ed + 0x2aa01d31;
         return state;
    }

private:
    uint32_t state = 0u;
};
