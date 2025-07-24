#include "shared.h"
#include "gs_config.h"

#include "audio/gs_audio.h"

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
