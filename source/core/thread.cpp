#include "shared.h"
#include "thread.h"

static constexpr const char* kThreadDefaultName = "UNAMED THREAD";

TestDriveThread::TestDriveThread( const char* pThreadName /*= nullptr */ )
    : stackSize( 0x3000 )
#if defined( OTDU_WIN32 )
    , threadID( 0 )
    , hThread( 0 )
#elif defined( OTDU_MACOS )
    , threadHandle( nullptr )
#endif
{
    setName( pThreadName );
}

TestDriveThread::~TestDriveThread()
{
}

void TestDriveThread::terminate()
{
#if defined( OTDU_WIN32 )
    if ( hThread != nullptr ) {
        DWORD exitCode = 0;
        BOOL bOperationResult = GetExitCodeThread( hThread, &exitCode );

        if ( bOperationResult ) {
            TerminateThread( hThread, 0 );
            CloseHandle( hThread );
        }

        hThread = nullptr;
        threadID = 0;
    }
#elif defined( OTDU_MACOS )
    if ( threadHandle != nullptr ) {
        pthread_join( threadHandle, nullptr );
        threadHandle = nullptr;
    }
#endif
}

bool TestDriveThread::initialize(  StartRoutine_t pThreadStartRoutine, void* lpParameter, const size_t threadStackSize, const uint32_t dwCreationFlags )
{
    if ( threadStackSize != 0 ) {
        stackSize = threadStackSize;
    }
#if defined( OTDU_WIN32 )
    hThread = CreateThread( 0, stackSize, pThreadStartRoutine, lpParameter, dwCreationFlags, &threadID );
    return hThread != 0;
#elif defined( OTDU_MACOS )
    pthread_attr_t attributes;

    int attrResult = pthread_attr_init( &attributes );
    OTDU_ASSERT( attrResult == 0 );

    pthread_attr_setstacksize( &attributes, stackSize );

    int result = pthread_create( &threadHandle, &attributes, pThreadStartRoutine, lpParameter );
    OTDU_ASSERT( result == 0 );

    pthread_attr_destroy( &attributes );

    return result == 0;
#endif
}

void TestDriveThread::setName( const char* pThreadName )
{
    name = std::string( ( pThreadName ) ? pThreadName : kThreadDefaultName );
}

bool TestDriveThread::setPriority( const eThreadPriority threadPriority )
{
#if defined( OTDU_WIN32 )
    BOOL BVar1 = 0;

    if ( hThread != ( HANDLE )0x0 ) {
        switch ( threadPriority ) {
        case TP_Highest:
            BVar1 = SetThreadPriority( hThread, THREAD_PRIORITY_HIGHEST );
            return BVar1 != 0;
        case TP_Normal:
            BVar1 = SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
            return BVar1 != 0;
        case TP_Low:
            BVar1 = SetThreadPriority( hThread, THREAD_PRIORITY_IDLE );
            return BVar1 != 0;
        case TP_Critical:
            BVar1 = SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );
            return BVar1 != 0;
        default:
            return threadPriority != TP_Highest;
        }
    }
#elif defined( OTDU_MACOS )
    return true; // TODO: AFAIK not possible
#endif

    return false;
}
