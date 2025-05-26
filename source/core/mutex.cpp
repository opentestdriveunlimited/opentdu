#include "shared.h"
#include "mutex.h"

#include "time.h"

static constexpr char* kMutexDefaultName = "UNAMED MUTEX";

TestDriveMutex::TestDriveMutex( const char* pMutexName /*= nullptr */ )
    : name( kMutexDefaultName )
#if defined( OTDU_WIN32 )
    , criticalSection()
    , lockedByThreadID( 0 )
#elif defined( OTDU_MACOS )
    , mutex( PTHREAD_MUTEX_INITIALIZER )
#endif
    , lockCount( 0 )
    , totalLockDurationInSeconds( 0.0f )
    , lockStartTimeInSeconds( 0.0f )
    , bLocked( 0 )
    , bInitialized( 0 )
    , bMarkedForDeletion( 0 )
{
    initialize( pMutexName );
}

TestDriveMutex::~TestDriveMutex()
{

}

void TestDriveMutex::initialize( const char* pMutexName /*= nullptr */ )
{
    if ( bInitialized ) {
        return;
    }
#ifdef OTDU_WIN32
    InitializeCriticalSection( &criticalSection );
#endif
    if ( pMutexName ) {
        name = std::string( pMutexName );
    }

    bInitialized = true;
}

void TestDriveMutex::terminate()
{
    if ( bMarkedForDeletion ) {
        return;
    }

    bMarkedForDeletion = true;
#if defined( OTDU_WIN32 )
    DeleteCriticalSection( &criticalSection );
#elif defined( OTDU_MACOS )
#endif
}

void TestDriveMutex::lock()
{
#if defined( OTDU_WIN32 )
    EnterCriticalSection( &criticalSection );
    lockedByThreadID = GetCurrentThreadId();
#elif defined( OTDU_MACOS )
    pthread_mutex_lock( &mutex );
#endif
    bLocked = true;
    lockCount++;
    lockStartTimeInSeconds = GetTimeSeconds();
}

void TestDriveMutex::unlock()
{
    float startTime = lockStartTimeInSeconds;
    float currentTime = GetTimeSeconds();

    lockStartTimeInSeconds = 0.0f;
    totalLockDurationInSeconds += ( currentTime - startTime );
#if defined( OTDU_WIN32 )
    LeaveCriticalSection( &criticalSection );
    lockedByThreadID = 0;
#elif defined( OTDU_MACOS )
    pthread_mutex_unlock( &mutex );
#endif
    bLocked = false;
}
