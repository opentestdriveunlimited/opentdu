#include "shared.h"
#include "mng_base.h"

Manager::Manager()
    : pPreviousManager( nullptr )
    , pNextManager( nullptr )
{

}

bool Manager::initializeAsync( TestDriveGameInstance* pInstance )
{
    this->initialize( pInstance );
    return false;
}
