#include "tdu_instance.h"

#include "core/arg_parser.h"
#include "core/logger.h"
#include "core/thread.h"
#include "core/mutex.h"
#include "core/event.h"
#include "core/locale.h"
#include "core/assert.h"

#include "system/gs_system.h"
#include "filesystem/gs_file.h"
#include "filesystem/gs_boot_datas.h"
#include "debug/gs_console.h"
#include "debug/gs_debug.h"
#include "debug/gs_profile.h"
#include "config/gs_config.h"
#include "render/gs_render.h"
#include "game/gs_intro_pool.h"
#include "database/gs_database.h"
#include "player_data/gs_playerdata.h"
#include "filesystem/gs_dirty_disk.h"
#include "flash/gs_flash.h"
#include "world/weather/gs_weather.h"
#include "world/gs_world.h"
#include "gs_timer.h"
#include "game/vehicle/gs_car_colors.h"
#include "audio/gs_audio.h"
#include "input/gs_device.h"
#include "movie/gs_movie.h"
#include "world/gs_world_streaming.h"
#include "audio/gs_vehicle_sound.h"
#include "game/vehicle/gs_car_common.h"
#include "render/postfx/gs_postfx.h"
#include "audio/gs_ambiance.h"
#include "game/gs_subtitles.h"
#include "game/gs_cinematics.h"
#include "physics/gs_physics.h"
#include "render/camera/gs_camera.h"
#include "game/vehicle/gs_car_state.h"
#include "game/vehicle/gs_car_display.h"
#include "world/gs_details.h"
#include "physics/gs_physicsfx.h"
#include "render/particles/gs_particles.h"
#include "game/avatar/gs_cloth_system.h"
#include "game/avatar/gs_avatar.h"
#include "world/gs_pedestrians.h"
#include "game/scenaric/gs_scenaric.h"
#include "gs_pub.h"
#include "world/gs_aircraft.h"
#include "world/gs_boat.h"
#include "ai/gs_ai.h"
#include "online/gs_online.h"
#include "game/gs_tutorial.h"
#include "game/vehicle/gs_brand_car.h"
#include "world/gs_mng_resources.h"
#include "system/gs_keyboard_pc.h"
#include "system/gs_pc_options.h"
#include "gs_cd_key.h"
#include "player_data/gs_radio.h"

#include "render/shaders/shader_register.h"

#include "game/car_showcase/gm_car_showcase.h"

const char** gpCmdLineArgs = nullptr;
int32_t gCmdLineArgCount = 0;
TestDriveGameInstance* gpTestDriveInstance = nullptr;

// TODO: Move those to the right cpp/h
static bool gWindowed = false;
static bool gNoPub = false;
static bool gFPSCounterEnabled = false;
static bool gPOSEnabled = false;
static int8_t gVBL = false;
static char gORBAddress[128];
static uint64_t gAvailableMemory = 0;

static constexpr char* kDefaultORBAddress = "orb.testdriveunlimited.com";

static CmdLineArg CmdLineArgWindowed( "w", []( const char* pArg ) { gWindowed = true; } );
static CmdLineArg CmdLineArgVBL( "vbl", []( const char* pArg ) {
    char var = *pArg;

    if ( 47 < var && var < 51 ) {
        gVBL = var - 48;
    }
} );
static CmdLineArg CmdLineArgFPS( "fps", []( const char* pArg ) { gFPSCounterEnabled = true; } );
static CmdLineArg CmdLineArgPOS( "pos", []( const char* pArg ) { gPOSEnabled = true; } );
static CmdLineArg CmdLineArgNoPub( "nopub", []( const char* pArg ) { gNoPub = true; } );
static CmdLineArg CmdLineArgORB( "orb", []( const char* pArg ) {
    strncpy( gORBAddress, pArg, sizeof( char ) * 128 );
    gORBAddress[127] = '\0';

    OTDU_LOG_INFO( "ORB set to '%s'\n", gORBAddress );
} );

typedef enum eErrorMessage {
    EM_DX9NotSupported = 0,
    EM_SoundCardNotDetected = 1,
    EM_NotEnoughMemory = 2
} eErrorMessage;

constexpr const char* kErrorMessageDX9NotSupported[L_Count] = {
    "_no trad",
    "Incorrect version of DirectX!",
    "Version de DirectX incorrecte !",
    "�Versi�n incorrecta de DirectX!",
    "Falsche DirektX-Version!",
    "Versione errata di DirectX!",
    "_no trad",
    "_no trad",
    "_no trad",
    "_no trad",
    "_no trad"
};

constexpr const char* kErrorMessageNotEnoughMemory[L_Count] = {
    "_no trad",
    "Not enough system memory!",
    "Pas assez de m�moire syst�me !",
    "�No bastante memoria de sistema!",
    "Nicht genug Systemged�chtnis!",
    "Non abbastanza memoria di sistema!",
    "_no trad",
    "_no trad",
    "_no trad",
    "_no trad",
    "_no trad"
};

static void DisplayErrorMsgLocalized( eErrorMessage errorType )
{
    eLocale systemLocale = GetLocaleFromSystem();

    const char* pErrorMessage = nullptr;
    switch ( errorType ) {
    case EM_DX9NotSupported:
        pErrorMessage = kErrorMessageDX9NotSupported[errorType];
        break;
    case EM_NotEnoughMemory:
        pErrorMessage = kErrorMessageNotEnoughMemory[errorType];
        break;
    }

    if ( pErrorMessage != nullptr ) {
        TestDrive::DisplayMessageBox( pErrorMessage );
    }
}

static bool HasEnoughSysMemAvail()
{
    gAvailableMemory = TestDrive::GetAvailableVirtualMemory();
    if ( gAvailableMemory < 484 ) {
        DisplayErrorMsgLocalized( EM_NotEnoughMemory );
        return false;
    }

    return true;
}

static void InstanciationWorker(TestDriveGameInstance::FileInstanciationWorkerInfos *param_1)
{
    do {
        while( true ) {
            param_1->Event.waitForEvent();
            param_1->pThread->setPriority(eThreadPriority::TP_Normal);
            
            if (param_1->pCurrentFile == nullptr) {
                break;
            }
            param_1->pCurrentFile->instanciate();
            param_1->bDone = true;
        }
        if (param_1->pNextFile != nullptr) {
            param_1->pNextFile->instanciate(param_1->pThread);
        }
        param_1->bDone = true;
    } while( true );
}

int32_t TestDrive::InitAndRun( const char** pCmdLineArgs, const int32_t argCount )
{
    strncpy( gORBAddress, kDefaultORBAddress, sizeof( char ) * 26 );

    gpCmdLineArgs = (const char**)( pCmdLineArgs );
    gCmdLineArgCount = argCount;

    gArgParserRegister.parseCmdLineArgs( gpCmdLineArgs, gCmdLineArgCount );

    TestDriveGameInstance gameInstance( pCmdLineArgs, argCount );
    if ( !IsAlreadyRunning() ) {
        bool bCheckSuccessful = HasEnoughSysMemAvail();

        if ( bCheckSuccessful ) {
            gameInstance.mainLoop();
            gameInstance.terminate();
            return 0;
        }
    }

    return 1;
}

const char* TestDrive::GetVersion()
{
#ifdef OTDU_DEVBUILD
    #define OTDU_BUILD_TYPE "D"
#else
    #define OTDU_BUILD_TYPE ""
#endif

#ifdef OTDU_WIN32
    #define OTDU_BUILD_OS "01"
#elif defined(OTDU_MACOS)
    #define OTDU_BUILD_OS "02"
#elif defined(OTDU_UNIX)
    #define OTDU_BUILD_OS "03"
#else
    #define OTDU_BUILD_OS "00"
#endif

#ifdef OTDU_OPENGL
#define OTDU_GFX_API "02"
#elif defined(OTDU_VULKAN)
#define OTDU_GFX_API "03"
#elif defined(OTDU_D3D9)
#define OTDU_GFX_API "01"
#else
#define OTDU_GFX_API "00"
#endif

#ifdef OTDU_CLANG
#define OTDU_COMPILER "02"
#elif defined(OTDU_MSVC)
#define OTDU_COMPILER "01"
#elif defined(OTDU_GCC)
#define OTDU_COMPILER "03"
#else
#define OTDU_COMPILER "00"
#endif

    return "OpenTDU" OTDU_BUILD_OS "" OTDU_COMPILER "" OTDU_GFX_API "" OTDU_COMMIT_HASH "" OTDU_BUILD_TYPE;
}

TestDriveGameInstance::TestDriveGameInstance( const char** argv, const int32_t argc )
    : numRegisteredServices( 0 )
    , argc( argc )
    , argv( argv )
    , bUseDVDVFS( false )
    , bNoAudio( false )
    , bNoSfx( false )
    , bProfile( false )
    , bMemCheck( false )
    , bFileTOC( false )
    , bSkip( false )
    , bPendingGameModeChange( false )
    , bRequestedExit( false )
    , wishedGameMode( GM_BootMenu )
    , memCheckSize( 0x2800 )
    , activeGameMode( GM_BootMenu )
    , previousGameMode( GM_Invalid )
    , mainLoopThread()
    , instanciationThread()
    , stackFileMutex( "StackInst" )
    , pUnkownResource( nullptr )
{
    registeredServices.reserve( 64 );

    instanciationThread.initialize((TestDriveThread::StartRoutine_t)InstanciationWorker, &fileInstanciation, 0x10000, 0);
    instanciationThread.setPriority(eThreadPriority::TP_Normal);

    fileInstanciation.pThread = &instanciationThread;

    mainLoopInfos.CoreIndex = 0;
    gpTestDriveInstance = this;
}

TestDriveGameInstance::~TestDriveGameInstance()
{
    bRequestedExit = true;

    for ( GameSystem* service : registeredServices ) {
        service->terminate();
        delete service;
    }
    registeredServices.clear();
}

void TestDriveGameInstance::terminate()
{
    instanciationThread.terminate();
    stackFileMutex.terminate();
    mainLoopThread.terminate();
}

void TestDriveGameInstance::mainLoop()
{
    bool bInitialized = initialize();
    if (!bInitialized) {
        OTDU_LOG_ERROR("Fatal: failed to initialize engine instance\n");
        return;
    }
    
    // Set localization for common flash
    uint32_t validGM = 0;
    uint32_t movieIndex = 0;
    bool bVar1 = gpFlash->getFlashMovieIndex(&validGM, &movieIndex, "GENERAL");
    if (bVar1) {
        MoviePlayer* pMVar2 = gpFlash->getMoviePlayer(validGM, movieIndex);
        if (pMVar2 != nullptr && pMVar2->MovieBank.isLoaded() && pMVar2->MovieBank.getFlashPlayer() != nullptr) {
            uint32_t uVar4 = 0;
            FlashPlayer* peVar3 = gpFlash->getFlashPlayer("GENERAL");
            gpDatabase->setFlashLocalization(uVar4, 0, peVar3);
        }
    }
    
    bVar1 = gpFlash->getFlashMovieIndex(&validGM, &movieIndex, "CONFIGPC");
    if (bVar1) {
        MoviePlayer* pMVar2 = gpFlash->getMoviePlayer(validGM, movieIndex);
        if (pMVar2 != nullptr && pMVar2->MovieBank.isLoaded() && pMVar2->MovieBank.getFlashPlayer() != nullptr) {
            uint32_t uVar4 = 0;
            FlashPlayer* peVar3 = gpFlash->getFlashPlayer("CONFIGPC");
            gpDatabase->setFlashLocalization(uVar4, 0, peVar3);
        }
    }
    
    // gpOnGSFileInit = &LAB_009a3360;
    if (activeGameMode != eGameMode::GM_Login) {
        OTDU_UNIMPLEMENTED;
        // GSCarDisplay::FreeResources((GSCarDisplay *)&gGSCarDisplay);
        // pSVar4 = &(gGSWorld.pStreamingManager)->SpotStream;
        // ((gGSWorld.pStreamingManager)->SpotStream).bStream = true;
        // pSVar4->bDraw = true;
    }

    float deltaTime = 0.0f;
    
    GameMode* pGVar6 = nullptr;

    while ( !bRequestedExit ) {
        switch ( activeGameMode ) {
        case GM_CarShowCase:
            pGVar6 = new ( TestDrive::Alloc( sizeof( GMCarShowcase ) ) ) GMCarShowcase();
            break;
        default:
            OTDU_FATAL_ERROR("GAME MODE '%s' (%u) IS NOT IMPLEMENTED!\n", kGameModeNames[activeGameMode], activeGameMode);
            break;
        }

        OTDU_LOG_INFO("Switching to game mode '%s'...\n", kGameModeNames[activeGameMode]);
        pGVar6->mainLoop( this );

        // if (pGameMode != (GMAITrainer *)0x0) {
        //   (*((GMBase_vtable *)(pGameMode->super).super.super.lpVtbl)->~GameMode)(pGameMode,true);
        // }
        // goto switchD_0097ce90_caseD_3;

        for ( GameSystem* service : registeredServices ) {
            service->tick( deltaTime );
        }
    }

    OTDU_LOG_DEBUG( "Exit requested\n" );
}

void TestDriveGameInstance::setGameMode( eGameMode newGameMode )
{
    wishedGameMode = newGameMode;
}

bool TestDriveGameInstance::initialize()
{
    OTDU_LOG_INFO("Initializing OpenTDU version '%s'...\n", TestDrive::GetVersion());

    bool baseServicesInit = initializeBaseServices();
    if ( !baseServicesInit ) {
        return false;
    }

    gpBootDatas->loadDataBanks();
    gpFile->flushPendingAsync();

    bool gameServicesInit = initializeGameServices();
    if ( !gameServicesInit ) {
        return false;
    }

    for (GameSystem* pSystem : registeredServices) {
        pSystem->reset();
    }

    return true;
}

bool TestDriveGameInstance::initializeBaseServices()
{
    bool operationResult = true;

    OTDU_ASSERT( operationResult &= registerService<GSSystem>() );
    OTDU_ASSERT( operationResult &= registerService<GSFile>() );
    OTDU_ASSERT( operationResult &= registerService<GSConsole>() );
    OTDU_ASSERT( operationResult &= registerService<GSTimer>() );
    OTDU_ASSERT( operationResult &= registerService<GSConfig>() );
    // NOTE: Has to be initialized early (to register world listeners)
    // The original code does this late (as I believe the listeners list is static and/or
    // GSWeather is statically allocated)
    OTDU_ASSERT( operationResult &= registerService<GSWeather>() );
    OTDU_ASSERT( operationResult &= registerService<GSRender>() );
    OTDU_ASSERT( operationResult &= registerService<GSProfile>() );
    OTDU_ASSERT( operationResult &= registerService<GSDevice>() );
    OTDU_ASSERT( operationResult &= registerService<GSDebug>() );
    OTDU_ASSERT( operationResult &= registerService<GSAudio>() );
    OTDU_ASSERT( operationResult &= registerService<GSFlash>() );
    OTDU_ASSERT( operationResult &= registerService<GSMovie>() );
    gpConfig->parseIniFiles();
    OTDU_ASSERT( operationResult &= registerService<GSBootDatas>() );

    gShaderRegister.registerMasterTable();

    return operationResult;
}

bool TestDriveGameInstance::initializeGameServices()
{
    bool operationResult = true;
    OTDU_ASSERT( operationResult &= registerService<GSIntroPool>() );
    OTDU_ASSERT( operationResult &= registerService<GSWorldStreaming>() );
    OTDU_ASSERT( operationResult &= registerService<GSVehicleSound>() );
    OTDU_ASSERT( operationResult &= registerService<GSCarCommon>() );
    OTDU_ASSERT( operationResult &= registerService<GSCarColors>() );
    OTDU_ASSERT( operationResult &= registerService<GSPostFX>() );
    OTDU_ASSERT( operationResult &= registerService<GSAmbiance>() );
    OTDU_ASSERT( operationResult &= registerService<GSSubtitles>() );
    OTDU_ASSERT( operationResult &= registerService<GSCinematics>() );
    OTDU_ASSERT( operationResult &= registerService<GSPhysics>() );
    OTDU_ASSERT( operationResult &= registerService<GSCamera>() );
    OTDU_ASSERT( operationResult &= registerService<GSCarState>() );
    OTDU_ASSERT( operationResult &= registerService<GSCarDisplay>() );
    OTDU_ASSERT( operationResult &= registerService<GSDatabase>() );
    OTDU_ASSERT( operationResult &= registerService<GSDetails>() );
    OTDU_ASSERT( operationResult &= registerService<GSPhysicsFX>() );
    OTDU_ASSERT( operationResult &= registerService<GSParticles>() );
    OTDU_ASSERT( operationResult &= registerService<GSWorld>() );
    OTDU_ASSERT( operationResult &= registerService<GSClothSystem>() );
    OTDU_ASSERT( operationResult &= registerService<GSAvatar>() );
    OTDU_ASSERT( operationResult &= registerService<GSPedestrians>() );
    OTDU_ASSERT( operationResult &= registerService<GSPub>() );
    OTDU_ASSERT( operationResult &= registerService<GSScenaric>() );
    OTDU_ASSERT( operationResult &= registerService<GSAircraft>() );
    OTDU_ASSERT( operationResult &= registerService<GSBoat>() );
    OTDU_ASSERT( operationResult &= registerService<GSAI>() );
    OTDU_ASSERT( operationResult &= registerService<GSOnline>() );
    OTDU_ASSERT( operationResult &= registerService<GSTutorial>() );
    OTDU_ASSERT( operationResult &= registerService<GSPlayerData>() );
    OTDU_ASSERT( operationResult &= registerService<GSBrandCar>() );
    OTDU_ASSERT( operationResult &= registerService<GSMngResources>() );
    OTDU_ASSERT( operationResult &= registerService<GSKeyboardPC>() );
    OTDU_ASSERT( operationResult &= registerService<GSDirtyDisk>() );
    OTDU_ASSERT( operationResult &= registerService<GSRadio>() );

    // gpConfig->parseRadioIni();
    // gpConfig->parseLeaderboardIni();

    OTDU_ASSERT( operationResult &= registerService<GSPCOptions>() );
    OTDU_ASSERT( operationResult &= registerService<GSCDKey>() );

    return operationResult;
}

template<typename TGS>
bool TestDriveGameInstance::registerService()
{
    TGS* pSystem = new TGS();
    if ( !pSystem->initialize( this ) ) {
        delete pSystem;
        return false;
    }

    OTDU_LOG_INFO( "Succesfully registered service '%s'\n", pSystem->getName() );

    registeredServices.push_back( pSystem );
    return true;
}

void TestDriveGameInstance::setNextGameMode( eGameMode nextGameMode )
{
    if ( bPendingGameModeChange ) {
        OTDU_LOG_WARN( "Failed to submit GM change: GM change already in progress!\n" );
        return;
    }

    if ( activeGameMode != nextGameMode ) {
        previousGameMode = activeGameMode;
    }

    activeGameMode = nextGameMode;
}

void TestDriveGameInstance::flushPendingFileInstanciation(bool param_2)
{
    ScopedMutexLock mutexLock(&stackFileMutex);

    uint32_t uVar2 = pendingStreamedResources.size();
    if (0 < pendingStreamedResources.size()) {
        uint32_t iVar3 = 0;
        if (param_2) {
            iVar3 = uVar2 - 1;
        }

        while (iVar3 < uVar2 && uVar2 != 0) {
            updateFileInstanciation();
            uVar2 = pendingStreamedResources.size();
        }
         
        bool bVar1 = fileInstanciation.bDone;
        while (!bVar1) {
            ThreadYield();
            bVar1 = fileInstanciation.bDone;
        }
    }

    if (!param_2) {
        bool bVar2 = fileInstanciation.bDone;
        while (!bVar2) {
            ThreadYield();
            bVar2 = fileInstanciation.bDone;
        }
    }
}

void TestDriveGameInstance::updateFileInstanciation()
{
    bool bVar1 = fileInstanciation.bDone;
    if (pendingStreamedResources.size() < 1) {
        if (bVar1) {
            fileInstanciation.pCurrentFile = nullptr;
            fileInstanciation.bDone = false;
            fileInstanciation.Event.reset();
        }
    } else if (bVar1) {
        fileInstanciation.pCurrentFile = pendingStreamedResources.front();
        fileInstanciation.bDone = false;
        fileInstanciation.Event.reset();

        pendingStreamedResources.pop_front();
        fileInstanciation.pCurrentFile->markAsInstanciated();
    }
}
