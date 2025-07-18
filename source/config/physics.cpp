#include "shared.h"
#include "gs_config.h"

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
