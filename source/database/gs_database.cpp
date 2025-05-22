#include "shared.h"
#include "gs_database.h"

GSDatabase* gpDatabase = nullptr;

GSDatabase::GSDatabase()
    : NbChallengeNames( 0xa0 )
    , NbChallengeDesc( 0x38 )
    , bInitialized( false )
{
    gpDatabase = this;
}

GSDatabase::~GSDatabase()
{

}

bool GSDatabase::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSDatabase::tick()
{

}

void GSDatabase::terminate()
{

}
