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
#include "console/gs_console.h"
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
#include "render/shaders/shader_register.h"

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

            /*TestDrive::MainLoop( &testDriveInstance, pCmdLineArgs, &TestDrive_vtable );
            testDriveInstance.lpVtbl = ( TestDrive_vtable* )&TestDrive_vtable;
            TestDrive::Term( &testDriveInstance );*/
            return 0;
        }
    }

    return 1;
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
    , pMainLoopThread()
    , pInstanciationThread()
    , pStackFileMutex( "StackInst" )
{
    registeredServices.reserve( 64 );

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

static char gActiveVideoName[259];

void TestDriveGameInstance::mainLoop()
{
    bool initialized = initialize();
    OTDU_ASSERT( initialized );

    float deltaTime = 0.0f;
    while ( !bRequestedExit ) {
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
    bool baseServicesInit = initializeBaseServices();
    if ( !baseServicesInit ) {
        return false;
    }

    bool gameServicesInit = initializeGameServices();
    if ( !gameServicesInit ) {
        return false;
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
    OTDU_ASSERT( operationResult &= registerService<GSWorld>() );
    OTDU_ASSERT( operationResult &= registerService<GSWeather>() );
    OTDU_ASSERT( operationResult &= registerService<GSRender>() );
    OTDU_ASSERT( operationResult &= registerService<GSFlash>() );
    OTDU_ASSERT( operationResult &= registerService<GSDirtyDisk>() );
    
    gShaderRegister.registerMasterTable();

    return operationResult;
}

bool TestDriveGameInstance::initializeGameServices()
{
    bool operationResult = true;
    OTDU_ASSERT( operationResult &= registerService<GSIntroPool>() );
    OTDU_ASSERT( operationResult &= registerService<GSBootDatas>() );
    OTDU_ASSERT( operationResult &= registerService<GSPlayerData>() );
    OTDU_ASSERT( operationResult &= registerService<GSDatabase>() );
    OTDU_ASSERT( operationResult &= gCarColors.initialize( this ) );
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
