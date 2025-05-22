#include "shared.h"
#include "gs_intro_pool.h"

GSIntroPool::GSIntroPool()
    : pIntroPool( nullptr )
{

}

GSIntroPool::~GSIntroPool()
{

}

bool GSIntroPool::initialize( TestDriveGameInstance* pGameInstance )
{
    //MOV        EAX, [UINT_00f7f16c] = 330986Ch
    //ADD        puVar1, DAT_00a80000 = 24h    $
    static constexpr uint32_t kPoolSize = 64526444;

    bPaused = false;
    pIntroPool = TestDrive::AllocAligned( kPoolSize );
    return true;
}

void GSIntroPool::terminate()
{
    if ( pIntroPool != nullptr ) {
        TestDrive::FreeAligned( pIntroPool );
        pIntroPool = nullptr;
    }
}
