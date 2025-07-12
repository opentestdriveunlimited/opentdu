#pragma once

#include "game/gs_base.h"
#include "core/mutex.h"
#include "core/locale.h"
#include "core/notifier.h"

class GSPlayerData : public GameSystem, public Notifiable {
public:
    const char* getName() const override { return "Service : PlayerData"; }

public:
    GSPlayerData();
    ~GSPlayerData();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void setDefaultLanguage( const char* pDefaultLanguage, bool bForceInit = false );
    void setDefaultSeatPosition( const float height, const float depth );

    void setLODQuality( const int32_t newValue );

    const char* getLanguage();

    void setUnitSystem( const bool bUseImperial );

private:
    uint8_t bInitialized : 1;
    uint8_t bForceUpdate : 1;
    uint8_t bUseImperialUnits : 1;
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
    int32_t lodQuality;

    std::string languageStr;
    
    TestDriveMutex* pMutex;

private:
    void updateUnitSystem();
};

extern GSPlayerData* gpPlayerData;
