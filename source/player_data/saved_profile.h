#pragma once

#include "core/notifier.h"

class SavedProfile : public Notifiable
{
public:
    SavedProfile();
    ~SavedProfile();

private:
    uint32_t profileIndex;
    uint32_t clubIndex;
    uint32_t clubSpotIndex;
    uint32_t money;
    uint32_t numCoupons;
    int32_t pendingMoneyOperation;
    float moneyOperationTimer;
    uint16_t currentCarIndex;

    uint32_t tutorialState;
    uint32_t unknownDword;
    uint32_t unknownDword2;
    
    uint64_t currentHouseHashcode;
    char pNickname[32];

    uint32_t lastNewsIndex;
    float level;
    float odometer;
    char pClubName[16];
    
    uint8_t bUnknownFlag : 1;
};
