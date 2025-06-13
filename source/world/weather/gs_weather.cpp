#include "shared.h"
#include "gs_weather.h"

GSWeather* gpWeather = nullptr;

GSWeather::GSWeather()
    : GameSystem()
    , bEnabled( true )
    , bFreezeTime( false )
    , bUpdateTimerEnabled( false )
    , timeScale( 50.0f )
    , currentTimeOfDay( 0u )
    , timeRefreshInterval{ 0.0f, 30.0f, 0.0f }
{
    OTDU_ASSERT( gpWeather == nullptr );
    gpWeather = this;
}

GSWeather::~GSWeather()
{

}

bool GSWeather::initialize( TestDriveGameInstance* pGameInstance )
{
    return false;
}

void GSWeather::tick(float deltaTime)
{

}

void GSWeather::terminate()
{

}

void GSWeather::registerListener(WorldListener *pListener)
{
    registeredListeners.push_back( pListener );
}

void GSWeather::updateActiveConfig()
{
}
