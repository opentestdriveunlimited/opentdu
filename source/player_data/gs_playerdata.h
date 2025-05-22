#pragma once

#include "game/gs_base.h"
#include "core/mutex.h"

// TODO: Move to a separate header
struct PlayerDataLanguage {
    union {
        uint16_t ID;
        char Str[2];
    };

    constexpr PlayerDataLanguage( const uint16_t langID ) : ID( langID ) {}
    PlayerDataLanguage( const char* langStr ) { Str[0] = langStr[0]; Str[1] = langStr[1]; }
};

static bool operator == ( PlayerDataLanguage l, PlayerDataLanguage r )
{
    return l.ID == r.ID;
}

static constexpr PlayerDataLanguage kLangUS( 0x7573 );
static constexpr PlayerDataLanguage kLangSP( 0x7370 );
static constexpr PlayerDataLanguage kLangKO( 0x6b6f );
static constexpr PlayerDataLanguage kLangJP( 0x6a61 );

class GSPlayerData : public GameSystem {
public:
    const char* getName() const override { return "Service : PlayerData"; }

public:
    GSPlayerData();
    ~GSPlayerData();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

    void setDefaultLanguage( const char* pDefaultLanguage, bool bUnknownBoolean = false );
    void setDefaultSeatPosition( const float height, const float depth );

private:
    uint8_t bInitialized : 1;
    uint8_t bUnknownBoolLanguage : 1;
    uint8_t bUseUSFontPage : 1;
    uint8_t bTutorialCompleted : 1;
    bool bOnlineOps[9];
    uint8_t bUnusedFlag : 1;

    float padSensitivity;
    float barSensitivity;
    float mouseSensitivity;
    float timeout;

    float seatPositionHeight;
    float seatPositionPosition;

    uint32_t resolutionWidth;
    uint32_t resolutionHeight;
    uint32_t refreshRate;

    PlayerDataLanguage language;
    
    TestDriveMutex* pMutex;

private:
    void updateFontPage();
    void loadPage( const bool bLoadDefaultFontPage );
};

extern GSPlayerData* gpPlayerData;
