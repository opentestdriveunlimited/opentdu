#include "shared.h"
#include "gs_playerdata.h"

#include "flash/gs_flash.h"
#include "game/mng_hud.h"
#include "input/gs_device.h"
#include "config/gs_config.h"

#include "core/mutex.h"
#include "core/locale.h"
#include "core/ed_store.h"

#include "tdu_instance.h"

GSPlayerData* gpPlayerData = nullptr;

GSPlayerData::GSPlayerData()
    : GameSystem()
    , Notifiable()
    , bInitialized( false )
    , bForceUpdate( false )
    , bUseImperialUnits( true )
    , bTutorialCompleted( false )
    , bIsOnline( false )
    , padSensitivity( -7.0f * 0.06666667f )
    , barSensitivity( 0.5f )
    , mouseSensitivity( 0.5f )
    , seatPositionHeight( 0.0f )
    , seatPositionPosition( 0.0f )
    , resolutionWidth( 1280 )
    , resolutionHeight( 720 )
    , refreshRate( 60 )
    , lodQuality( 2 )
    , antiAliasing( eAntiAliasingMethod::AAM_Disabled )
    , bHDREnabled( false )
    , languageStr( "us" )
    , timeout( 0.0f )
    , pMutex( new TestDriveMutex() )
{
    pMutex->initialize();
    //bOnlineOps[0] = DAT_014d5608 == 0;
    //bOnlineOps[1] = true;
    //bOnlineOps[2] = false;
    //bOnlineOps[3] = true;
    //bOnlineOps[4] = true;
    //bOnlineOps[5] = true;
    //bOnlineOps[6] = true;
    //bOnlineOps[7] = true;
    //bOnlineOps[8] = true;

    //{
    //    std::unique_lock lock( parameterMutex );

    //    FUN_008980c0( param_1->Language );
    //    if ( PTR_011403d0 != ( undefined* )0x0 ) {
    //        FUN_00896bd0( &param_1->field_0x184, PTR_011403d0 );
    //    }

    //    timeout = -144000.1;
    //    bUnusedFlag = true;
    //}

    gpPlayerData = this;
}

GSPlayerData::~GSPlayerData()
{

}

bool GSPlayerData::initialize( TestDriveGameInstance* )
{
    if ( bInitialized ) {
        return true;
    }
    padSensitivity = ( static_cast<float>( gDefaultSensitivity ) - 1.0f ) * 0.06666667f;

    return true;
}

void GSPlayerData::tick(float totalTime, float deltaTime)
{

}

void GSPlayerData::terminate()
{

}

void GSPlayerData::setDefaultLanguage( const char* pDefaultLanguage, bool bForceInit )
{
    languageStr = pDefaultLanguage;
    bForceUpdate = bForceInit;

    if ( gpTestDriveInstance && gpTestDriveInstance->getActiveGameMode() != GM_Login ) {
        updateUnitSystem();
    }
}

void GSPlayerData::setDefaultSeatPosition( const float height, const float depth )
{

}

void GSPlayerData::setLODQuality( const int32_t newValue )
{
    OTDU_ASSERT( newValue > 0 && newValue < 2 );
    lodQuality = newValue;
}

const char *GSPlayerData::getLanguage()
{
    return languageStr.empty() ? "us" : languageStr.c_str();
}

eAntiAliasingMethod GSPlayerData::getAAMethod() const
{
    return antiAliasing;
}

bool GSPlayerData::isHDREnabled() const
{
    return bHDREnabled;
}

void GSPlayerData::updateUnitSystem()
{
    bool bUseImperialUnits = (languageStr == "us");
    setUnitSystem( bUseImperialUnits );
}

void GSPlayerData::setUnitSystem( const bool bUseImperial )
{
    bUseImperialUnits = bUseImperial;
    if ( gpMngHud != nullptr ) {
        gpMngHud->updateUnitSystem();
    }

    MoviePlayer* pMoviePlayer = gpFlash->getMoviePlayer( 0, 1 );
    if (pMoviePlayer != nullptr && pMoviePlayer->MovieBank.getState() == BankFlash::eBankFlashInit::BFI_Loaded) {
        FlashPlayer* pFlashPlayer = pMoviePlayer->MovieBank.getFlashPlayer();
        if (pFlashPlayer != nullptr) {
            uint32_t flashUnitsFlash = bUseImperialUnits ? 1u : 0u;
            pFlashPlayer->setVariableValue( "/:state_Metric", flashUnitsFlash );
        }
    }
}

bool GSPlayerData::isOnline() const
{
    return bIsOnline;
}

void GSPlayerData::FUN_008985d0(void *pExternalBuffer, int bufferLength)
{
    // FUN_008985d0
    ScopedMutexLock lock(pMutex);
    
    edStore local_30;
    local_30.bindExternalBuffer(pExternalBuffer, bufferLength);
    local_30.clear();
    local_30.writeInteger(0x34);

    OTDU_UNIMPLEMENTED;
}
