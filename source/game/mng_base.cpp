#include "shared.h"
#include "mng_base.h"

Manager::Manager()
    : pPreviousManager( nullptr )
    , pNextManager( nullptr )
    , index( 0 )
{

}

bool Manager::initializeAsync( TestDriveGameInstance* pInstance )
{
    this->initialize( pInstance );
    return false;
}

uint32_t Manager::getIndex() const
{
    return index;
}
