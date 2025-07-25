#pragma once

#include "game/mng_base.h"

class FlashPlayer;

class MngFlashLocalize : public Manager {
public:
    static char VersionString[64];

public:
    const char* getName() const override { return "MngFlashLocalize"; }

public:
    MngFlashLocalize();
    ~MngFlashLocalize();

    virtual bool initialize( TestDriveGameInstance* ) override;
    void reset();

    int32_t FUN_00716f60(uint64_t param_1);
    void FUN_00717ff0(FlashPlayer* param_2, uint64_t param_3, uint64_t param_4, uint32_t param_5);

private:
    std::vector<std::tuple<std::string, std::string>> localizedStrings;
    std::vector<std::tuple<std::string, std::string>> menuLocalizedStrings;
    int32_t unknown;

private:
    void resetMenuLocalization();
};

extern MngFlashLocalize* gpMngFlashLocalize;
