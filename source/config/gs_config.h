#pragma once

#include "game/gs_base.h"
#include "render/gs_render_helper.h"

enum class eFullscreenMode {
    FM_Auto = 0,
    FM_Manual,
    FM_Desktop
};

enum eTimerPriority : uint8_t {
    RealTime = 0,
    FixedTime = 1,
    SyncToVBL = 2,
};

class GSConfig : public GameSystem {
public:
    const char* getName() const override { return "Service : Config"; }
    inline const char* getResRootPath() const { return bLoadFromDVD ? pDVDPath : pResPath; }
    inline const char* getAssetQualityFolder() const { return WindowWidth > 640 ? "HiRes" : "LowRes"; }

public:
    float FrametimeTargetMS;
    eFullscreenMode FullscreenMode;

    char pResPath[128];
    char pDVDPath[128];

    char pLevelName[32];
    int32_t StartAreaRow;
    int32_t StartAreaCol;
    int32_t StartHeightmapRow;
    int32_t StartHeightmapCol;

    char pDefaultCarConfig[3][32];

    uint16_t Width;
    uint16_t Height;
    uint16_t WindowWidth;
    uint16_t WindowHeight;

    int32_t NbMaxInstance;
    int32_t NbMaxHierNode;

    int32_t NbMaxPlayers;
    int32_t NbMaxActionsPerPlayer;
    int32_t NbMaxChannelPerActions;
    eAntiAliasingMethod AntiAliasing;

    int32_t NbVBL;
    int32_t RefreshRateInHz;
    int32_t TimerPriority;

    uint8_t b169Ratio : 1;
    uint8_t bWindowed : 1;
    uint8_t bInvertMouse : 1;
    uint8_t bEnableMovie : 1;
    uint8_t bAutoConnectFreeride : 1;
    uint8_t bEnableHDRI : 1;
    uint8_t bDefaultCarConfig : 1;
    uint8_t bKeepAlive : 1;
    uint8_t bHDRWeather : 1;
    uint8_t bMemCheck : 1;
    uint8_t bSearchInBootFirst : 1;
    uint8_t bLoadFromDVD : 1;
    uint8_t bEnableMainMenu : 1;
                            
public:
    GSConfig();
    ~GSConfig();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void setScreenRatio( const bool bIsUsing169Ratio );

private:
    using CommandCallback_t = std::function<void( const char* )>;

    std::unordered_map<uint32_t, CommandCallback_t> registeredCommands;

    char pSystemPCIniPath[OTDU_MAX_PATH];

private:
    void registerCommands();

    void registerGameCommands();
    void registerSystemCommands();
    void registerDeviceCommands();
    void registerReplayCommands();
    void registerAudioCommands();
    void registerRadioCommands();
    void registerPhysicsCommands();
    void registerFXCommands();

    void registerCommand( const char* pCommandName, CommandCallback_t pCommandCallback );
    void registerCommandFloat( const char* pCommandName, float* pVariable );
    void registerCommandInt( const char* pCommandName, int32_t* pVariable );
    void registerCommandUInt( const char* pCommandName, uint32_t* pVariable );
    void registerCommandInt( const char* pCommandName, int16_t* pVariable );
    void registerCommandUInt( const char* pCommandName, uint16_t* pVariable );
    void parseSyncToVBL( const char* pArg );
};

extern GSConfig* gpConfig;
