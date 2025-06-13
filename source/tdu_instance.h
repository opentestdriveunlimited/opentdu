#pragma once

#include "shared.h"

#include "core/thread.h"
#include "core/event.h"
#include "core/mutex.h"

#include "game/gm_base.h"

class GameSystem;

static uint32_t gDesktopWidth = 0;
static uint32_t gDesktopHeight = 0;
static uint32_t gDesktopRefreshRate = 0;
static bool gbNvidiaGPU = false;

namespace TestDrive {
    int32_t InitAndRun( const char** pCmdLineArgs, const int32_t argCount );
}

class TestDriveThread;
class TestDriveEvent;
class TestDriveMutex;

class TestDriveGameInstance {
public:
    struct MainLoopInfos {
        TestDriveEvent* pEvent;
        int32_t CoreIndex;
        float DeltaTime;
        float Time;
        uint8_t bDone : 1;
        uint8_t bDirtyServices : 1;
        uint8_t bDirtyManagers : 1;

        MainLoopInfos()
            : pEvent( new TestDriveEvent() )
            , CoreIndex( 0 )
            , DeltaTime( 0.0f )
            , Time( 0.0f )
            , bDone( false )
            , bDirtyServices( false )
            , bDirtyManagers( false )
        {

        }

        ~MainLoopInfos()
        {
            delete pEvent;
        }
    };

    struct FileInstanciationWorkerInfos {
        TestDriveEvent* pEvent;
        int32_t CoreIndex;
        float DeltaTime;
        float Time;
        uint8_t bDone : 1;
        uint8_t bDirtyServices : 1;
        uint8_t bDirtyManagers : 1;

        FileInstanciationWorkerInfos()
            : pEvent( new TestDriveEvent() )
            , CoreIndex( 0 )
            , DeltaTime( 0.0f )
            , Time( 0.0f )
            , bDone( false )
            , bDirtyServices( false )
            , bDirtyManagers( false )
        {

        }

        ~FileInstanciationWorkerInfos()
        {
            delete pEvent;
        }
    };

public:
    inline eGameMode getActiveGameMode() const { return activeGameMode; }
    inline void requestExit() { bRequestedExit = true; }
    inline bool hasRequestedExit() const { return bRequestedExit; }

public:
    TestDriveGameInstance( const char** argv, const int32_t argc );
    ~TestDriveGameInstance();

    void mainLoop();
    void setGameMode( eGameMode newGameMode );
    void setNextGameMode( eGameMode nextGameMode );

private:
    std::vector< GameSystem* > registeredServices;
    uint32_t numRegisteredServices;

    uint32_t argc;
    const char** argv;

    uint8_t bUseDVDVFS : 1;
    uint8_t bNoAudio : 1;
    uint8_t bNoSfx : 1;
    uint8_t bProfile : 1;
    uint8_t bMemCheck : 1;
    uint8_t bFileTOC : 1;
    uint8_t bSkip : 1;
    uint8_t bPendingGameModeChange : 1;
    uint8_t bRequestedExit : 1;

    eGameMode wishedGameMode;
    uint32_t memCheckSize;
    eGameMode activeGameMode;
    eGameMode previousGameMode;

    TestDriveThread pMainLoopThread;
    TestDriveThread pInstanciationThread;

    TestDriveMutex pStackFileMutex;

    MainLoopInfos mainLoopInfos;

private:
    bool initialize();
    bool initializeBaseServices();
    bool initializeGameServices();

    template<typename TGS>
    bool registerService();
};

extern TestDriveGameInstance* gpTestDriveInstance;

static bool gbLoadingInProgress = false; // TODO: Where does it belong?

