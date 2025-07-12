#pragma once

class DrawList;

typedef enum eGameMode : uint32_t {
    GM_BootMenu = 0,
    GM_FreeRide = 1,
    GM_Map = 2,
    GM_Home = 3,
    GM_Shop = 4,
    GM_Mission = 5,
    GM_GameModeEdit = 6,
    GM_CarShowCase = 7,
    GM_InstantChallenge = 8,
    GM_TestCar = 9,
    GM_Garage = 10,
    GM_Living = 11,
    GM_Sandbox = 12,
    GM_Goodies = 13,
    GM_AvatarShowcase = 14,
    GM_DriveIn = 15,
    GM_Dressing = 16,
    GM_AfterMarket = 17,
    GM_VideoBumper = 18,
    GM_AITrainer = 19,
    GM_CinViewer = 20,
    GM_Estates = 21,
    GM_Club = 22,
    GM_Lobby = 23,
    GM_Login = 24,
    GM_OnlineReplay = 25,
    GM_Intro = 26,
    GM_OutroDemo = 27,
    GM_BoatTour = 28,
    GM_Jail = 29,
    GM_IntroDemo = 30,
    GM_Count = 31,

    GM_Invalid = 0xffffffff
} eGameMode;

static constexpr const char* kGameModeNames[eGameMode::GM_Count] = {
    "BOOTMENU",
    "FREE RIDE",
    "MAP",
    "SHOP",
    "MISSION",
    "GameModeEdit",
    "CAR SHOWCASE",
    "InstantChallenge",
    "TestCar",
    "GARAGE",
    "LIVING",
    "SandBox",
    "Goodies",
    "AVATAR SHOWCASE",
    "DRIVE IN",
    "DRESSING",
    "AFTER MARKET",
    "VIDEO BUMPER",
    "AI TRAINER",
    "CIN VIEWER",
    "ESTATES",
    "CLUB",
    "LOBBY",
    "LOGIN",
    "ONLINEREPLAY",
    "INTRO",
    "OUTRO DEMO",
    "BOATTOUR",
    "JAIL",
    "INTRO DEMO"
};

class GameMode {
public:
    inline eGameMode getCurrentGameMode() const { return currentGameMode; }

public:
    GameMode();
    virtual ~GameMode();

    void mainLoop();

protected:
    uint32_t    transitionFlags;
    float       transitionTime;
    uint32_t    ticksPerLoop;
    eGameMode   currentGameMode;
    DrawList*   pTransitionDrawList;

    uint8_t     bExitRequested : 1;
    uint8_t     bMessageBoxVisible : 1;
    uint8_t     bAsync : 1;
    uint8_t     bLoadAsync : 1;
    uint8_t     bDraw : 1;
};

extern GameMode* gpActiveGameMode;
