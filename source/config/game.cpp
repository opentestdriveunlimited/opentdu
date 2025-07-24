#include "shared.h"
#include "gs_config.h"

#include "core/hash/crc32.h"

#include "flash/gs_flash.h"
#include "database/gs_database.h"
#include "game/scenaric/gs_scenaric.h"

#include "online/gs_online.h"
#include "tdu_instance.h"
#include "player_data/gs_playerdata.h"

#include "game/game_cheats.h"

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
