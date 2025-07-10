#include "shared.h"
#include "flash_sounds.h"

FlashSounds::FlashSounds()
    : Bank()
    , pMemoryPool( nullptr )
{
}

FlashSounds::~FlashSounds()
{
}

bool FlashSounds::initialize( void* pMemory )
{
    pMemoryPool = pMemory;
    OTDU_UNIMPLEMENTED;
    return true;
}
