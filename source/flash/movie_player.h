#pragma once

#include "bank_flash.h"

class FlashEntity;

struct MoviePlayer {
    BankFlash MovieBank;
    FlashEntity* pEntityInstance;

    int32_t Level;

    uint8_t bInputActive : 1;
    uint8_t bPaused : 1;
    uint8_t bEnabled : 1;
    uint8_t bShouldDraw : 1;

    MoviePlayer()
        : pEntityInstance( nullptr )
        , Level( -1 )
        , bInputActive( false )
        , bPaused( false )
        , bEnabled( false )
        , bShouldDraw( false )
    {
        
    }
};
