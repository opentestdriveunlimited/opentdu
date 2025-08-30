#pragma once
#pragma once

#ifdef OTDU_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOIME
#define NOGDICAPMASKS
#define NOMENUS
#define NOKEYSTATES
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#define NOMINMAX
#include <windows.h>

#include <timeapi.h>
#include <WinUser.h>

// TODO: Figure out who owns this...
extern HINSTANCE gHINSTANCE;
extern int32_t gNCmdShow;

#define OTDU_MAX_PATH MAX_PATH

#elif defined( OTDU_UNIX )
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#define OTDU_MAX_PATH PATH_MAX
#endif

namespace TestDrive
{
#if defined( OTDU_WIN32 )
    // TODO: For now we'll stick with what CRT offers (probably good enough)
    inline void* Alloc( const size_t size )
    {
        return malloc( size );
    }

    inline void Free( void* pMemory )
    {
        free( pMemory );
    }

    inline void* AllocAligned( const size_t size, const uint32_t alignment = 0x10 )
    {
        return _aligned_malloc( size, alignment );
    }

    inline void FreeAligned( void* pMemory )
    {
        _aligned_free( pMemory );
    }

    inline void GetWorkingDir( char* pBuffer, const size_t size )
    {
        GetCurrentDirectoryA( static_cast< DWORD >( size ), pBuffer );
    }

    inline bool IsAlreadyRunning()
    {
        CreateMutexA( ( LPSECURITY_ATTRIBUTES )0x0, 0, "TestDriveUnlimited" );
        HRESULT mutexCreationResult = GetLastError();
        return ( mutexCreationResult == ERROR_ALREADY_EXISTS );
    }

    inline uint64_t GetAvailableVirtualMemory()
    {
        MEMORYSTATUSEX memoryStatus = { 0 };
        memoryStatus.dwLength = 64;

        GlobalMemoryStatusEx( &memoryStatus );
        return memoryStatus.ullTotalPhys;
    }

    inline void DisplayMessageBox( const char* pErrorMessage, const char* pWindowTitle = "Test Drive Unlimited" )
    {
        MessageBoxA( 0, pErrorMessage, pWindowTitle, 0 );
    }
#elif defined( OTDU_UNIX )
    inline void* Alloc( const size_t size )
    {
        return malloc( size );
    }

    inline void Free( void* pMemory )
    {
        free( pMemory );
    }

    // TODO: For now we'll stick with what CRT offers (probably good enough)
    inline void* AllocAligned( const size_t size, const uint32_t alignment = 0x10 )
    {
        return aligned_alloc( size, alignment );
    }

    inline void FreeAligned( void* pMemory )
    {
        free( pMemory );
    }

    inline void GetWorkingDir( char* pBuffer, const size_t size )
    {
        getcwd( pBuffer, size );
    }

    inline bool IsAlreadyRunning()
    {
        int pid_file = open( "/tmp/opentdu.pid", O_CREAT | O_RDWR, 0666 );
        int rc = flock( pid_file, LOCK_EX | LOCK_NB );
        if ( rc ) {
            return ( EWOULDBLOCK == errno );
        }

        return false;
    }

    inline uint64_t GetAvailableVirtualMemory()
    {
#if defined( OTDU_MACOS )
        size_t pagesz = sysconf( _SC_PAGE_SIZE );
        uint64_t mem = 0ull;
        size_t len = sizeof( mem );
        sysctlbyname( "hw.memsize", &mem, &len, NULL, 0 );
        return mem;
#else
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
#endif
    }

    inline void DisplayMessageBox( const char* pErrorMessage, const char* pWindowTitle = "Test Drive Unlimited" )
    {
        OTDU_UNIMPLEMENTED;
    }
#endif
}
