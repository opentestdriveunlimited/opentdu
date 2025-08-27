#pragma once

#include "game/gs_base.h"
#include "core/mutex.h"
#include "core/locale.h"
#include "core/notifier.h"

#include "render/gs_render_helper.h"

class GSPlayerData : public GameSystem, public Notifiable {
public:
    const char* getName() const override { return "Service : PlayerData"; }

public:
    GSPlayerData();
    ~GSPlayerData();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;

    void setDefaultLanguage( const char* pDefaultLanguage, bool bForceInit = false );
    void setDefaultSeatPosition( const float height, const float depth );

    void setLODQuality( const int32_t newValue );

    const char* getLanguage();
    eAntiAliasingMethod getAAMethod() const;
    bool isHDREnabled() const;

    void setUnitSystem( const bool bUseImperial );
    bool isOnline() const;

    void FUN_008985d0(void *pExternalBuffer,int bufferLength);
    bool FUN_00897990();

private:
    uint8_t bInitialized : 1;
    uint8_t bForceUpdate : 1;
    uint8_t bUseImperialUnits : 1;
    uint8_t bTutorialCompleted : 1;
    bool bOnlineOps[9];
    uint8_t bUnusedFlag : 1;
    uint8_t bIsOnline : 1;
    uint8_t field526850_0x967d4 : 1;
    uint8_t field526864_0x967e5 : 1;
    int32_t field526854_0x967d8;

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

    eAntiAliasingMethod antiAliasing;
    uint8_t bHDREnabled : 1;
    std::string languageStr;
    
    TestDriveMutex* pMutex;

private:
    void updateUnitSystem();
};

extern GSPlayerData* gpPlayerData;
