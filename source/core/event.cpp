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
#elif defined( OTDU_MACOS )
        sem_destroy( &semaphore );
#endif
        semaphore = NULL;
    }
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
    sem_init(&semaphore, 0, 0);
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
    return sem_wait(&semaphore) == 0;
#endif
}
