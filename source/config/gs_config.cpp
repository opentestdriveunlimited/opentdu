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

#include "tdu_instance.h"

#include "city.h"

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
    , pSystemPCIniPath( "SystemPC.ini" )
{
    OTDU_ASSERT( gpConfig == nullptr );
    gpConfig = this;
}

GSConfig::~GSConfig()
{

}

bool GSConfig::initialize( TestDriveGameInstance* )
{
    registerCommands();

    ParseArgValueFromCmdLine( "system", pSystemPCIniPath, OTDU_MAX_PATH );

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

    return true;
}

void GSConfig::tick()
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

// GamePC.ini
void GSConfig::registerGameCommands()
{
    registerCommand( "SETPROTOCOLVERSION", [&]( const char* pArg ) {
        TestDrive::CRC::Protocol = TestDriveCRC32( pArg );
    } );

    registerCommand( "SETFREERIDEVERSION", [&]( const char* pArg ) {
        gpOnline->setFreeRideVersion( pArg );
    } );

    registerCommand( "SETDEFAULTLANGUAGE", [&]( const char* pArg ) { gpPlayerData->setDefaultLanguage( pArg ); } );

    registerCommandInt( "NBOFCHALLENGENAMES", &gpDatabase->NbChallengeNames );
    registerCommandInt( "NBOFCHALLENGEDESC", &gpDatabase->NbChallengeDesc );

    registerCommand( "UNLOCKALLROADS", [&]( const char* pArg ) { gbUnlockAllRoads = true; } );
    registerCommand( "DELOCKCHALLENGESROAD", [&]( const char* pArg ) { gpScenaric->bIsRoadLocked = false; } );

    registerCommand( "SETRESPATH", [&]( const char* pArg ) {
        strcpy( pResPath, pArg );
    } );

    registerCommand( "SETDVDPATH", [&]( const char* pArg ) {
        strcpy( pDVDPath, pArg );
    } );

    registerCommand( "SEARCHINBOOTFIRST", [&]( const char* pArg ) {
        // TRUE or FALSE
        bSearchInBootFirst = ( strcmp( pArg, "TRUE" ) == 0 );
    } );

    registerCommand( "ADDLEVEL", [&]( const char* pArg ) {
        strcpy( pLevelName, pArg );
    } );
    registerCommandInt( "SETSTARTAREAROW", &StartAreaRow );
    registerCommandInt( "SETSTARTAREACOL", &StartAreaCol );
    registerCommandInt( "SETSTARTHEIGHTMAPROW", &StartHeightmapRow );
    registerCommandInt( "SETSTARTHEIGHTMAPCOL", &StartHeightmapCol );

    registerCommand( "SETCURGAMEMODE", [&]( const char* pArg ) {
        if ( gpTestDriveInstance ) {
            eGameMode gameMode = ( eGameMode )atoi( pArg );
            gpTestDriveInstance->setGameMode( gameMode );
        }
    } );
    
    registerCommand( "CHALL_TIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CHALL_SPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CHALL_RACE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NB_TIMES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NB_SPEEDS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NB_RACES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETFLASHCOMMONPATH", [&]( const char* pArg ) {
        gpFlash->setCommonPath( pArg );
    } );

    registerCommand( "SETFLASHROOTPATH", [&]( const char* pArg ) {
        gpFlash->setRootPath( pArg );
    } );

    registerCommand( "BOOTMENU", [&]( const char* pArg ) {
        bEnableMainMenu = ( strcmp( pArg, "TRUE" ) == 0 );
    } );

    registerCommand( "BOOTMENULIST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ADDFLASHMOVIE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETMAXFLASHMOVIES", [&]( const char* pArg ) {
        int32_t moviesCount = atoi( pArg );
        gpFlash->setMaxFlashMovies( moviesCount );
    } );

    registerCommandUInt( "SETTRACESIZE", &gpScenaric->TraceSize );

    registerCommand( "SETAUTOSAVEBOOKMARKLENGTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ENABLEMINIMAPDENSITY", [&]( const char* pArg ) {
        gEnableMinimapDensity = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 );
    } );

    registerCommand( "ENABLEMINIMAPIA", [&]( const char* pArg ) {
        gEnableMinimapAI = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 );
    } );

    registerCommand( "ENABLEMINIMAPNETPLAYER", [&]( const char* pArg ) {
        gEnableMinimapNetPlayers = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 );
    } );

    registerCommand( "CUBEMAPSHOOTING", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HIGHRESSCREENENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SIZEINCREASE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SAMPLEMINSPACE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PIXSAMPLINGWIDTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "VEHICLEMAXCOUNT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TRAFFICMAXCOUNT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PARKEDMAXCOUNT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "POLICEMAXCOUNT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PHYSICSMAXCOUNT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TRACEMODEENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OPPONENTDIFFICULTY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "IAREPLACEPLAYER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FORCENBOPPONENTS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SAMECARTHANPLAYER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OPPSHARPTURNLIMIT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OPPSHARPTURNMAXSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OPPSHARPTURNCLAMPVALUE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DISABLEPATHOPT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DISABLEOBSTACLEPARSER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WANTEDDIFFICULTY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WANTEDBEHAVIOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "POLICEFORCECHASE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "POLICEFORCEDENSITY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ENABLEJAIL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CULLINGCONFIG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SPAWNCONFIG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SPAWNDISTANCES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FORCECARMODEL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ENABLEBOTS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTCARCONFIG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTGARAGECONFIG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTHOUSE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTMONEY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALLHOUSESWITHDEFCARS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SERVER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ORB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTLOGIN", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LOBBYING", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GPSTRACKER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "KEEPALIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "COMDEBUG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AUTO_CONNECT_FREERIDE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CANSTARTONLINERACEALONE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_QOSRETIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_QOSGAMEOBJECTS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_CORRECTION_ACTIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_CORRECTION_DT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_RETIME_DT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_DRIVING_LOOPBACK_ACTIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LAN_DRIVING_LOOPBACK_SIZE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_QOSRETIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_QOSGAMEOBJECTS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_CORRECTION_ACTIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_CORRECTION_DT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_RETIME_DT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_DRIVING_LOOPBACK_ACTIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET512_DRIVING_LOOPBACK_SIZE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NET_CORRECTION_DISTRIBUTION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "MULTIPLAYERDETAILS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEMO_MODE_TIMER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "XBLMODESECURE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTSTARTPOSITION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REMOVESPOTFROMMAP", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TUTORIAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCENARIOTUTORIAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LOGCHALLENGESINFO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "INTROSTATE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PUB_BANDWIDTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PUB_VISIBILITY_MAX", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LOADIGE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GMEDESIGNER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ICESOLO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALLCHALLENGESSOLO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "MAPSCREENSHOTSMNGENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WOOSHENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "POSTFXENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NBELTULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NBELTNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NBELTFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHASTARTULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHALENGTHULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALESTARTULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALELENGTHULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALEFACTORULTRANEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHASTARTNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHALENGTHNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALESTARTNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALELENGTHNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALEFACTORNEAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHASTARTFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHALENGTHFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALESTARTFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALELENGTHFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SCALEFACTORFAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ALPHAREF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HEIGHTGRASS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WIDTHRATIOGRASS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SIZEDELTAGRASS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HEIGHTBUSHES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WIDTHRATIOBUSHES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SIZEDELTABUSHES", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WEATHERENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETDEFAULTWEATHERDESC", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETWEATHERDESC", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TIMESCALE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NAME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STATICCFG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DYNAMICCFG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WEATHERCONFIGNAME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GLOBALAMBIENTCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNAMBIENTCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNDIFFUSECOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNSPECULARCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNDIR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNDIRT2ANGLE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OCEANAMPLITUDE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WINDSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WINDDIR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINCOVER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINSHARPNESS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINOPACIFIER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINSCALEU", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINSCALEV", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINALPHASTART", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSPERLINALPHALENGTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FOGSTART", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FOGEND", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FOGMAX", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FOGCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNMARKERSIZE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNCORECOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNZSURFCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNHALOCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNCOEFF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNCOEFFHALO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNDAZZLETHRESHOLD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNDAZZLEDECREASESPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DOME2DB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DOMEEMISSIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDSEMISSIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNEMISSIVE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FILTER2DB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUN2DB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNCORE2DB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SUNHALO2DB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HEIGHTMAPCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HEIGHTMAPCONTRAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CONTRAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DESATURATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "BALANCE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDNAME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUDCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUD3DNAME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CLOUD3DCOLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARGLOBALFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARGLOBALENV", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARLUMMIN", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARLUMMAX", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARAMBIANTFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARDIFFUSEFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARSPECULARFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARREFLECTIONFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARREFLECTIONLUM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CARCLOUDSGROUNDFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "OCEANENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GRASSENABLED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "DEFAULTCINEMATIC", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}

// SystemPC.ini
void GSConfig::registerSystemCommands()
{
    registerCommand( "SETVIDEO", [&]( const char* pArg ) {
        // PAL50, PAL60, NTSC or NTSCJ
        if ( strncmp( pArg, "PAL60", strlen( "PAL60" ) ) == 0 ) {
            Width = 640;
            Height = 480;
            RefreshRateInHz = 60;
        } else if ( strncmp( pArg, "PAL50", strlen( "PAL50" ) ) == 0 ) {
            Width = 640;
            Height = 480;
            RefreshRateInHz = 50;
        } else if ( strncmp( pArg, "NTSC", strlen( "NTSC" ) ) == 0 ) {
            Width = 640;
            Height = 480;
            RefreshRateInHz = 60;
        } else if ( strncmp( pArg, "NTSCJ", strlen( "NTSCJ" ) ) == 0 ) {
            Width = 640;
            Height = 480;
            RefreshRateInHz = 60;
        }

        FrametimeTargetMS = 1.0f / static_cast< float >( RefreshRateInHz );
    } );

    registerCommand( "SETSCREENRATIO", [&]( const char* pArg ) {
        // RATIO_16_9 or RATIO_4_3
        setScreenRatio( strncmp( pArg, "RATIO_16_9", strlen( "RATIO_16_9" ) ) == 0 );
    } );

    registerCommand( "SETWINDOWED", [&]( const char* pArg ) {
        // TRUE or FALSE
        bWindowed = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0);
    } );

    registerCommand( "SETANTIALIAS", [&]( const char* pArg ) {
        // 4 or 2
        if ( strncmp( pArg, "4", 1 ) == 0 ) {
            AntiAliasing = eAntiAliasingMethod::AAM_MSAA_X4;
        } else if ( strncmp( pArg, "2", 1 ) == 0 ) {
            AntiAliasing = eAntiAliasingMethod::AAM_MSAA_X2;
        } else {
            OTDU_LOG_WARN( "Unknown antialiasing method '%s'\n", pArg );
            AntiAliasing = eAntiAliasingMethod::AAM_Disabled;
        }
    } );

    registerCommand( "SETHDANTIALIAS", [&]( const char* pArg ) {
        // 4 or 2
        if ( strncmp( pArg, "4", 1 ) == 0 ) {
            AntiAliasing = eAntiAliasingMethod::AAM_MSAA_X4;
        } else if ( strncmp( pArg, "2", 1 ) == 0 ) {
            AntiAliasing = eAntiAliasingMethod::AAM_MSAA_X2;
        } else {
            OTDU_LOG_WARN( "Unknown antialiasing method '%s'\n", pArg );
            AntiAliasing = eAntiAliasingMethod::AAM_Disabled;
        }
    } );

    
    
    registerCommandUInt( "SETWIDTH", &Width );
    registerCommandUInt( "SETHEIGHT", &Height );

    registerCommandUInt( "SETWINWIDTH", &WindowWidth );
    registerCommandUInt( "SETWINHEIGHT", &WindowHeight );

    registerCommand( "SETFULLSCREENRESOLUTION", [&]( const char* pArg ) {
        // AUTO, DESKTOP or MANUAL
        if ( strncmp( pArg, "AUTO", strlen( "AUTO" ) ) == 0 ) {
            FullscreenMode = eFullscreenMode::FM_Auto;
        } else if ( strncmp( pArg, "DESKTOP", strlen( "DESKTOP" ) ) == 0 ) {
            FullscreenMode = eFullscreenMode::FM_Desktop;
        } else if ( strncmp( pArg, "MANUAL", strlen( "MANUAL" ) ) == 0 ) {
            FullscreenMode = eFullscreenMode::FM_Manual;
        } else {
            OTDU_LOG_WARN( "Unknown fullscreen mode '%s'\n", pArg );
        }
    } );

    registerCommand( "SETHDSYNCTOVBL", std::bind( &GSConfig::parseSyncToVBL, this, std::placeholders::_1 ) );

    registerCommandInt( "SETHDNBVBL", &NbVBL );
    registerCommandUInt( "SETHDWIDTH", &Width );
    registerCommandUInt( "SETHDHEIGHT", &Height );

    registerCommandUInt( "NBMAXKEYBOARDS", &gNbMaxKeyboards );
    registerCommandUInt( "NBMAXMICE", &gNbMaxMice );
    registerCommandUInt( "NBMAXJOYSTICKS", &gNbMaxJoysticks );

    registerCommand( "NBMAXACTIONSPERPLAYER", [&]( const char* pArg ) {
        NbMaxActionsPerPlayer = atoi( pArg );

        gNbMaxActions = NbMaxActionsPerPlayer * NbMaxPlayers;
        OTDU_UNIMPLEMENTED;
        //// TODO: Actions array resize
        //if ( 0 < local_4 ) {
        //    iVar5 = 0;
        //    do {
        //        *( int* )( &this->pConfig->Actions->field_0x130 + iVar5 ) = param_2->IntegerValue;
        //        pTVar1 = this->pConfig->Actions;
        //        puVar2 = ( undefined4* )
        //            ( **( code** )( *( int* )gMainWindow._148_4_ + 0x20 ) )
        //            ( *( int* )( &pTVar1->field_0x130 + iVar5 ) << 2, 0, 0, 0 );
        //        *( undefined4** )( &pTVar1->field_0x134 + iVar5 ) = puVar2;
        //        for ( uVar3 = *( uint* )( &pTVar1->field_0x130 + iVar5 ) & 0x3fffffff; uVar3 != 0;
        //              uVar3 = uVar3 - 1 ) {
        //            *puVar2 = 0;
        //            puVar2 = puVar2 + 1;
        //        }
        //        for ( iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1 ) {
        //            *( undefined* )puVar2 = 0;
        //            puVar2 = ( undefined4* )( ( int )puVar2 + 1 );
        //        }
        //        iVar5 = iVar5 + 0x138;
        //        local_4 = local_4 + -1;
        //    } while ( local_4 != 0 );
        //}
        //OTDU_UNIMPLEMENTED
    } );

    registerCommand( "NMAXCHANNELPERACTIONS", [&]( const char* pArg ) {
        NbMaxChannelPerActions = atoi( pArg );
        gNbMaxActions = NbMaxChannelPerActions * NbMaxActionsPerPlayer * NbMaxPlayers;
    } );

    registerCommand( "NBMAXPLAYERS", [&]( const char* pArg ) {
        NbMaxPlayers = atoi( pArg );
        gNbMaxActions = NbMaxChannelPerActions * NbMaxActionsPerPlayer * NbMaxPlayers;
    } );

    registerCommand( "SETTIMER", [&]( const char* pArg ) {
        // REAL_TIME or FIXED_TIME
        if ( strncmp( "REAL_TIME", pArg, strlen( "REAL_TIME" ) ) == 0 ) {
            TimerPriority &= ~eTimerPriority::FixedTime;
        } else if ( strncmp( "FIXED_TIME", pArg, strlen( "FIXED_TIME" ) ) == 0 ) {
            TimerPriority |= eTimerPriority::FixedTime;
        } else {
            OTDU_LOG_WARN( "Unknown timer mode '%s'\n", pArg );
        }
    } );

    registerCommand( "SETSYNCTOVBL", std::bind( &GSConfig::parseSyncToVBL, this, std::placeholders::_1 ) );
    registerCommandInt( "SETNBVBL", &NbVBL );

    registerCommandInt( "SETNBMAXINSTANCE", &NbMaxInstance );
    registerCommandInt( "SETNBMAXHIERNODE", &NbMaxHierNode );

    registerCommand( "SETHDRI", [&]( const char* pArg ) {
        bEnableHDRI = atoi( pArg ) != 0;
    } );

    registerCommand( "SETMOVIE", [&]( const char* pArg ) {
        bEnableMovie = ( strncmp( pArg, "ON", strlen( "ON" ) ) == 0);
    } );

    // Used by the original game only (NOP in OTDU)
    registerCommand( "SETPROFILE", [&]( const char* pArg ) {
        // TRUE or FALSE
        // gpTestDriveInstance->bProfile = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 );
    } );

    registerCommand( "SETMEMCHECK", [&]( const char* pArg ) {
        // TRUE or FALSE
        // gpTestDriveInstance->bMemCheck = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0);
    } );

    registerCommand( "SETMEMCHECKSIZE", [&]( const char* pArg ) {
        // gpTestDriveInstance->memCheckSize = atoi( pArg ) << 10;
    } );
}

void GSConfig::registerDeviceCommands()
{
    registerCommand( "ADDACTION", [&]( const char* pArg ) {
        //AddAction	"CAR_ACCEL"				"ALL"	 1	"A"		"Keyboard"	"0"		"UP"
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ADDACTIONAND", [&]( const char* pArg ) {
        //AddAction	"CAR_ACCEL"				"ALL"	 1	"A"		"Keyboard"	"0"		"UP"
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETINVERTMOUSE", [&]( const char* pArg ) {
        bInvertMouse = ( strncmp( pArg, "TRUE", strlen( "TRUE" ) ) == 0 );
    } );

    registerCommand( "ACTIONSEXCLUSIONS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommandFloat( "FFB_FACTOR", &gFFBFactor );

    registerCommand( "FFB_SLIPANGLE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FFB_SPEEDFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FFB_LOADFROMACCEL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
        // TODO: Parsing
        // Format:
        // FFB_LoadFromAccel		1.0		0.9		0.8
        // 
        // gFFBLoadFromAccel[]
    } );

    registerCommand( "FFB_LOADFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
        // TODO: Parsing
        // Format:
        // FFB_LoadFactor		1.0		0.9		0.8
        // 
        // gFFBLoadFactor[]
    } );

    registerCommandFloat( "STEERINGDEADZONE", &gSteeringDeadZone );
    registerCommandFloat( "BRAKEDEADZONE", &gBrakeDeadZone );
    registerCommandFloat( "STEERINGTURN", &gSteeringTurn );
    registerCommandFloat( "STEERINGLINEARITY", &gSteeringLinearity );
    registerCommand( "CAMERADEADZONE", [&]( const char* pArg ) {
        gpCamera->setDeadZone( static_cast< float >( atof( pArg ) ) );
    } );

    registerCommand( "INVERTACCEL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "INVERTBRAKE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PEDALSSEPARATED", [&]( const char* pArg ) {
        gPedalsSeparated = atoi( pArg ) != 0;
    } );

    registerCommand( "DEFAULTSENSITIVITY", [&]( const char* pArg ) {
        gDefaultSensitivity = atoi( pArg );
    } );

    registerCommand( "DEFAULTSEATPOSITION", [&]( const char* pArg ) {
       /* gpPlayerData->setDefaultSeatPosition()*/
        OTDU_UNIMPLEMENTED
    } );
}

void GSConfig::registerReplayCommands()
{
    registerCommand( "SAVEREPLAY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LOADREPLAY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_CONFIG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_FREQUENCY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SWAPLENGTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SWAPTIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_HEIGHTS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_WIDTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPEEDRATIO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_CARWHEELOFFSET", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_CAROFFSETPOS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_CAROFFSETTRG", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_FOVNORMAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_FOVZOOM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_FOVLARGE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_FOVCAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_DOFCAMNORMAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_DOFCAMZOOM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_DOFCAMLARGE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_DOFCAMFIXED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGMINPOS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGMAXPOS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGRATE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGDAMPING", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGFORCESCALE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGFORCEDEADZONE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGFORCECLAMPING", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_SPRINGTIMESCALE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_HELICO_SPEEDS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_HELICO_HEIGHT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_HELICO_FOV", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RPL_HELICO_AIM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}

void GSConfig::registerAudioCommands()
{
    registerCommandInt( "SETOUTPUTNBSPEAKERS", &gpAudio->NbSpeakers );
    registerCommandFloat( "SETOUTPUTSPATIALFACTOR", &gpAudio->OutputSpatialFactor );
    registerCommandFloat( "SETOUTPUTDOPPLERFACTOR", &gpAudio->OutputDopplerFactor );

    registerCommand( "ADDMIXER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ADDREVERB", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ADDCLIENT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ADDMIXPRESET", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETGAMEMODEMIX", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETVEHICLEREVERBMODULE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SETAMBIANCEREVERBMODULE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}

void GSConfig::registerRadioCommands()
{
    registerCommand( "ADDSTATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}

void GSConfig::registerPhysicsCommands()
{
    registerCommand( "REMOTEDEBUGGER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PHYSICSDEBUGINFO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GRASSASTARMAC", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "PHYSICSTHREAD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FULLVISIBILITYTEST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TORQUEPURCENTTOCONTACT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "USESTEERCONTROL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "USEAUTOMATIC", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "GRAVITYCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FRONTFRICTIONCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REARFRICTIONCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FRONTLATERALFRICTIONCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REARLATERALFRICTIONCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REARLNGBRAKEFRICTIONCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FRONTLATBRAKEFRICTIONCOEFSLOW", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REARLATBRAKEFRICTIONCOEFSLOW", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FRONTLATBRAKEFRICTIONCOEFFAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REARLATBRAKEFRICTIONCOEFFAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LATBRAKEFRICTIONSPEEDSLOW", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LATBRAKEFRICTIONSPEEDFAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RESISTIVECOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "RESISTIVELATERALCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "INERTIACOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "MASSCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "CGHEIGHTCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REBOUNDCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SPRINGCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "IGNITIONFADETIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SHIFTTIMEMANUAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SHIFTTIMESEQUENTIAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SHIFT2NDTO1ST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TORQUEINERTIAUP", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TORQUEINERTIADOWN", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ENGINEBRAKEPOWER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "REDLINEPURCENT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WHEELMAXSLIPRATIO", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "WHEELMAXSLIPANGLE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SLIPRATIOSTARTLOOSELATERAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "MAXLATERALLOSS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGHELPSTRAIGHT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGHELPTURN", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "SENSITIVTY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGFILTERSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGLIMIT", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGCENTERSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGSTRAIGHTSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "STEERINGCOUNTERSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ACTIVEABSSPEED", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ACTIVEABSVALUE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ELECTRONICPARAMS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ELECTRONICPARAMSVALUE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ELECTRONICPOWERCOEF", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ACTIVETCTRIGGER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "ACTIVETCVALUE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "TCSTRAIGHTTRIGGER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AEROSPEEDS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AEROLENGTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AEROWIDTH", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AERODENSITY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LEAN_SPEED_RANGE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LEAN_SPEED_VAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LEAN_CURVE_RANGE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LEAN_CURVE_VAL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "LEAN_HEAD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}

void GSConfig::registerFXCommands()
{
    registerCommand( "FXO_REFLECTIONBLURINESS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_SPECULARPOWER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_BLURINESS_ATTENUATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_FRESNEL_ATTENUATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_NMAP_LR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_NMAP_HR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_NMAP_SCALER", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_FRESNEL", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_USE_REFLECTION_MAP", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXO_REFLECTION_COLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXB_LUMTHRESHOLD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXB_BLURNBPASS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXB_BLURSIZE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXB_MODCOEFF0", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXB_MODCOEFF1", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_FADEIN_DELAY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_FADEOUT_DELAY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_LUMTHRESHOLD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_BLURSIZE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_MODCOEFF0", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXP_MODCOEFF1", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_GAMEMODE", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_MIN_AVG_LUM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_MAX_AVG_LUM", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_DO_ADAPTATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_ADAPTATION_TIME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_CONTRAST", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_MIDDLE_GRAY", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_SATURATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_GLOW_BLUR_NBPASS", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXHDRI_GLOW_BLUR_KERNEL_SZ", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HDRCONF_NAME", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HDRCONF_GLOW_MOD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HDRCONF_STAR_MOD", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HDRCONF_BRIGHT_THRESHOLD_GLOW", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "HDRCONF_BRIGHT_THRESHOLD_STAR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "FXREFL_BG_COLOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AVATAROUTCARLIGHTFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AVATARCONVERTIBLELIGHTFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );

    registerCommand( "AVATARINCARLIGHTFACTOR", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
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
