#include "shared.h"
#include "event.h"

static constexpr char* kEventDefaultName = "UNAMED EVENT";

TestDriveEvent::TestDriveEvent( const char* pEventName /*= nullptr */ )
    : semaphore( NULL )
    , name( kEventDefaultName )
    , bInitialized( 0 )
    , bMarkedForDeletion( 0 )
{
    initialize( pEventName );
}

TestDriveEvent::~TestDriveEvent()
{
    if ( !bMarkedForDeletion ) {
        destroy();
    }
}

void TestDriveEvent::destroy()
{
    bMarkedForDeletion = true;

    if ( semaphore ) {
#if defined( OTDU_WIN32 )
        CloseHandle( semaphore );
        semaphore = NULL;
#elif defined( OTDU_MACOS )
        semaphore = false;
#endif
    }
}

bool TestDriveEvent::reset()
{
    if ( semaphore ) {
#if defined( OTDU_WIN32 )
        BOOL BVar1 = ReleaseSemaphore(semaphore,1,(LPLONG)0x0);
        if (BVar1 != 0) {
            ::Sleep(0);
            return true;
        }
#elif defined( OTDU_MACOS )
        semaphore = false;
        return true;
#endif
    }

    return false;
}

void TestDriveEvent::initialize( const char* pEventName )
{
    if ( bInitialized ) {
        return;
    }

    if ( pEventName ) {
        name = std::string( pEventName );
    }

#if defined( OTDU_WIN32 )
    semaphore = CreateSemaphoreA( 0, 0, 0x7fff, 0 );
#elif defined( OTDU_MACOS )
    semaphore = true;
#endif

    bInitialized = true;
}

bool TestDriveEvent::waitForEvent()
{
#if defined( OTDU_WIN32 )
    DWORD DVar1;

    if ( semaphore == ( HANDLE )0x0 ) {
        return false;
    }

    DVar1 = WaitForSingleObject( semaphore, 0xffffffff );
    return DVar1 == 0;
#elif defined( OTDU_MACOS )
    while (semaphore.load()) {}
    return true;
#endif
}
