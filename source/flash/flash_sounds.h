#pragma once

#include "filesystem/bank.h"

class FlashSounds : public Bank {
public:
    FlashSounds();
    ~FlashSounds();

    bool initialize( void* pMemory );

private:
    void* pMemoryPool;
};
