#include "shared.h"
#include "gs_config.h"

#include "core/hash/crc32.h"
#include "core/logger.h"
#include "core/assert.h"
#include "core/arg_parser.h"

#include "online/gs_online.h"
#include "player_data/gs_playerdata.h"
#include "game/game_cheats.h"
#include "render/camera/gs_camera.h"
#include "game/scenaric/gs_scenaric.h"
#include "database/gs_database.h"
#include "input/gs_device.h"
#include "audio/gs_audio.h"
#include "flash/gs_flash.h"
#include "filesystem/gs_file.h"

#include "tdu_instance.h"

#include "city.h"

static constexpr const char* kSystemPCIniFilename = "SystemPC.ini"; // DAT_00f76524
static constexpr const char* kRadioIniFilename = "Radio.ini"; // DAT_00f76574

GSConfig* gpConfig = nullptr;

GSConfig::GSConfig()
    : FrametimeTargetMS( 0.02f )
    , FullscreenMode( eFullscreenMode::FM_Auto )
    , pResPath( "EURO/BNK/" )
    , pDVDPath( "DVDEURO/" )
    , pLevelName( "Level_0" )
    , StartAreaRow( 0 )
    , StartAreaCol( 0 )
    , StartHeightmapRow( 0 )
    , StartHeightmapCol( 0 )
    , Width( 640 )
    , Height( 480 )
    , WindowWidth( 640 )
    , WindowHeight( 480 )
    , NbMaxInstance( 5000 )
    , NbMaxHierNode( 5000 )
    , NbMaxPlayers( 0 )
    , NbMaxActionsPerPlayer( 0 )
    , NbMaxChannelPerActions( 0 )
    , AntiAliasing( eAntiAliasingMethod::AAM_Disabled )
    , NbVBL( 1 )
    , RefreshRateInHz( 50 )
    , TimerPriority( 0 )
    , b169Ratio( false )
    , bWindowed( true )
    , bInvertMouse( false )
    , bEnableMovie( true )
    , bAutoConnectFreeride( false )
    , bEnableHDRI( false )
    , bDefaultCarConfig( false )
    , bKeepAlive( true )
    , bHDRWeather( false )
    , bMemCheck( false )
    , bSearchInBootFirst( true )
    , bLoadFromDVD( false )
    , bEnableMainMenu( true )
    , systemPCIniPath( kSystemPCIniFilename )
    , radioIniPath( "" ) // Dynamically built in FUN_0098dbb0
{
    OTDU_ASSERT( gpConfig == nullptr );
    gpConfig = this;

    systemPCIniPath.resize( OTDU_MAX_PATH );
    radioIniPath.resize( OTDU_MAX_PATH );
}

GSConfig::~GSConfig()
{

}

bool GSConfig::initialize( TestDriveGameInstance* )
{
    registerCommands();

    ParseArgValueFromCmdLine( "system", systemPCIniPath.data(), OTDU_MAX_PATH );

    return true;
}

void GSConfig::tick(float deltaTime)
{
    // NOP
}

void GSConfig::terminate()
{

}

void GSConfig::setScreenRatio( const bool bIsUsing169Ratio )
{
    b169Ratio = bIsUsing169Ratio;
}

void GSConfig::parseIniFiles()
{
    //// TODO: This is a bit crude; ideally we want to open the stream using the VFS instead of explicit std fstream
    //std::ifstream inputStream( pSystemPCIniPath, std::ios_base::in );
    //OTDU_ASSERT( inputStream.good() );

    //std::string inputLine;
    //while ( getline( inputStream, inputLine ) ) {
    //    int32_t lineIt = 0;
    //    // Trim tabs and white spaces. 
    //    // Only do this for the begining of the line; as the syntax is EXTREMELY inconsistent 
    //    // (they can use 'key = value' or 'key value'; therefore we need to keep white spaces and 
    //    // manually scrub to find the delimiter...)
    //    while ( ( inputLine[lineIt] == ' ' || inputLine[lineIt] == '\t' ) && inputLine[lineIt] != '\r' ) {
    //        lineIt++;
    //    }

    //    if ( inputLine.empty() || (lineIt == inputLine.size() - 1) ) {
    //        continue;
    //    }

    //    switch ( inputLine[lineIt] ) {
    //    case '[':
    //    {
    //        std::string sectionName;
    //        for ( int32_t i = lineIt + 1; i < inputLine.length(); i++ ) {
    //            if ( inputLine[i] == ']' ) {
    //                break;
    //            }
    //            sectionName += inputLine[i];
    //        }
    //        OTDU_LOG_DEBUG( "Found Section '%s'\n", sectionName.c_str() );
    //        break;
    //    }
    //    case '#':
    //    {
    //        // Comment; can skip line..
    //        OTDU_LOG_DEBUG( "Found Comment '%s'\n", &inputLine[lineIt + 1] );
    //        continue;
    //    }
    //    default:
    //    {
    //        std::string variableName;
    //        while ( (inputLine[lineIt] == ' ' || inputLine[lineIt] == '\t') && inputLine[lineIt] != '\r' ) {
    //            lineIt++;
    //        }
    //        while ( isalnum( inputLine[lineIt] ) ) {
    //            variableName += toupper( inputLine[lineIt] );
    //            lineIt++;
    //        }
    //        uint32_t cmdHash = CityHash32( variableName.c_str(), variableName.size() );
    //        auto it = registeredCommands.find( cmdHash );
    //        if ( it == registeredCommands.end() ) {
    //            OTDU_LOG_WARN( "Unknown command '%s' found in SystemPC.ini\n", variableName.c_str() );
    //            break;
    //        }
    //        OTDU_LOG_DEBUG( "Found Variable '%s' (hash: %p)\n", variableName.c_str(), cmdHash );

    //        // Find the start of the value declaration
    //        while ( inputLine[lineIt] == ' ' || inputLine[lineIt] == '\t' || inputLine[lineIt] == '=' ) {
    //            lineIt++;
    //        }

    //        // Invoke the parsing command
    //        it->second( &inputLine[lineIt] );
    //        break;
    //    }
    //    };
    //}
}

void GSConfig::registerCommands()
{
    registerSystemCommands();
    registerGameCommands();
    registerDeviceCommands();
    registerReplayCommands();
    registerAudioCommands();
    registerRadioCommands();
    registerPhysicsCommands();
    registerFXCommands();
}

void GSConfig::registerCommand( const char* pCommandName, CommandCallback_t pCommandCallback )
{
    uint32_t cmdHash = CityHash32( pCommandName, strlen( pCommandName ) );
    auto it = registeredCommands.find( cmdHash );
    OTDU_ASSERT_FATAL( it == registeredCommands.end() );

    registeredCommands.insert( std::make_pair( cmdHash, pCommandCallback ) );
}

static void SetVariableFloat( const char* pArg, float* pVariable )
{
    OTDU_ASSERT( pVariable );
    *pVariable = static_cast< float >( atof( pArg ) );
}

static void SetVariableInt32( const char* pArg, int32_t* pVariable )
{
    OTDU_ASSERT( pVariable );
    *pVariable = atoi( pArg );
}

static void SetVariableUInt32( const char* pArg, uint32_t* pVariable )
{
    OTDU_ASSERT( pVariable );
    *pVariable = static_cast< uint32_t >( atoi( pArg ) );
}

void GSConfig::registerCommandFloat( const char* pCommandName, float* pVariable )
{
    registerCommand( pCommandName, std::bind( &SetVariableFloat, std::placeholders::_1, pVariable ) );
}

void GSConfig::registerCommandInt( const char* pCommandName, int32_t* pVariable )
{
    registerCommand( pCommandName, std::bind( &SetVariableInt32, std::placeholders::_1, pVariable ) );
}

void GSConfig::registerCommandInt( const char* pCommandName, int16_t* pVariable )
{
    registerCommandInt( pCommandName, ( int32_t* )pVariable );
}

void GSConfig::registerCommandUInt( const char* pCommandName, uint32_t* pVariable )
{
    registerCommand( pCommandName, std::bind( &SetVariableUInt32, std::placeholders::_1, pVariable ) );
}

void GSConfig::registerCommandUInt( const char* pCommandName, uint16_t* pVariable )
{
    registerCommandUInt( pCommandName, ( uint32_t* )pVariable );
}

void GSConfig::parseSyncToVBL( const char* pArg )
{
    // TRUE or FALSE
    if ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 ) {
        TimerPriority |= eTimerPriority::SyncToVBL;
    } else {
        TimerPriority &= ~eTimerPriority::SyncToVBL;
    }
}

void GSConfig::parseRadioIni()
{
    const char* pResourcePath = getResRootPath();
    
    radioIniPath.clear();
    radioIniPath += pResourcePath;
    radioIniPath += "Sound/Radio/";
    radioIniPath += kRadioIniFilename;
    radioIniPath.resize(OTDU_MAX_PATH, '\0');

    ParseArgValueFromCmdLine( "radio", radioIniPath.data(), OTDU_MAX_PATH );

    char* pIniContent = nullptr;
    uint32_t iniContentSize = 0;
    if (!bSearchInBootFirst) {
        bool bFoundInVFS = gpFile->loadFile(radioIniPath.c_str(), &pIniContent, &iniContentSize);
    }
}
