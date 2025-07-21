#pragma once

#include "shared.h"

#include "core/thread.h"
#include "core/event.h"
#include "core/mutex.h"

#include "game/gm_base.h"

class GameSystem;
class TestDriveThread;
class TestDriveEvent;
class TestDriveMutex;
class StreamedResource;

namespace TestDrive {
    int32_t InitAndRun( const char** pCmdLineArgs, const int32_t argCount );
    const char* GetVersion();
}

class TestDriveGameInstance {
public:
    struct MainLoopInfos {
        TestDriveEvent Event;
        int32_t CoreIndex;
        float DeltaTime;
        float Time;
        uint8_t bDone : 1;
        uint8_t bDirtyServices : 1;
        uint8_t bDirtyManagers : 1;

        MainLoopInfos()
            : Event()
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
            Event.destroy();
        }
    };

    struct FileInstanciationWorkerInfos {
        TestDriveEvent Event;
        TestDriveThread* pThread;
        
        StreamedResource* pCurrentFile;
        StreamedResource* pNextFile;

        uint8_t bDone : 1;

        FileInstanciationWorkerInfos()
            : Event()
            , pThread( nullptr )
            , pCurrentFile( nullptr )
            , pNextFile( nullptr )
            , bDone( true )
        {

        }

        ~FileInstanciationWorkerInfos()
        {
            Event.destroy();
        }
    };

public:
    inline eGameMode getActiveGameMode() const { return activeGameMode; }
    inline void requestExit() { bRequestedExit = true; }
    inline bool hasRequestedExit() const { return bRequestedExit; }

public:
    TestDriveGameInstance( const char** argv, const int32_t argc );
    ~TestDriveGameInstance();

    void terminate();
    void mainLoop();
    void setGameMode( eGameMode newGameMode );
    void setNextGameMode( eGameMode nextGameMode );
    void switchToWishedGameMode();

    void flushPendingFileInstanciation(bool param_2);

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

    TestDriveThread mainLoopThread;
    MainLoopInfos mainLoopInfos;

    TestDriveThread instanciationThread;
    FileInstanciationWorkerInfos fileInstanciation;
    TestDriveMutex stackFileMutex;
    std::list<StreamedResource*> pendingStreamedResources;
    StreamedResource* pUnkownResource; // TODO: Why do they keep this pointer aside?

    uint32_t gameInitStep;

private:
    bool initialize();
    bool initializeBaseServices();
    bool initializeGameServices();

    template<typename TGS>
    bool registerService();

    void updateFileInstanciation();
};

extern TestDriveGameInstance* gpTestDriveInstance;

static bool gbLoadingInProgress = false; // TODO: Where does it belong?

