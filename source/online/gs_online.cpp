#include "shared.h"
#include "gs_online.h"

GSOnline* gpOnline = nullptr;

GSOnline::GSOnline()
    : pFreerideVersion( "PC Beta - 2.41A" )
{
    gpOnline = this;
}

GSOnline::~GSOnline()
{

}

bool GSOnline::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSOnline::tick(float totalTime, float deltaTime)
{

}

void GSOnline::terminate()
{

}

void GSOnline::setFreeRideVersion( const char* pVersion )
{
    strcpy( pFreerideVersion, pVersion );
}
