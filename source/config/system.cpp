#include "shared.h"
#include "gs_config.h"

#include "input/gs_device.h"

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
