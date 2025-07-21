#pragma once

#include "game/gs_base.h"

class GSCDKey : public GameSystem {
public:
    const char* getName() const override { return "NoName"; }

public:
    GSCDKey();
    ~GSCDKey();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;

    bool isValidKey(const char* pCDKey);

private:
    static constexpr uint32_t kKeyLength = 19; // 18 + null terminator

private:
    char pGameCDKey[kKeyLength];
    char pAddonCDKey[kKeyLength];

    uint8_t bValidGameKey : 1;
    uint8_t bValidAddonKey : 1;

private:
    void loadKeyFromCache();
    void writeKeyToCache();

    void updateGameKey(const char* param_1);
    bool checkAddonKey();
};

extern GSCDKey* gpCDKey;
