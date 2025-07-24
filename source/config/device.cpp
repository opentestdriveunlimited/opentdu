#include "shared.h"
#include "gs_config.h"

#include "input/gs_device.h"
#include "render/camera/gs_camera.h"

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
