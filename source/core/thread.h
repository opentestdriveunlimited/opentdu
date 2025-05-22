#pragma once

typedef enum eThreadPriority {
    TP_Highest = 0,
    TP_Normal = 1,
    TP_Low = 2,
    TP_Critical = 3
} eThreadPriority;

class TestDriveThread {
public:
#if defined( OTDU_WIN32 )
    using StartRoutine_t = LPTHREAD_START_ROUTINE;
#elif defined( OTDU_MACOS )
    using StartRoutine_t = void* (*)(void *);
#endif

public:
    TestDriveThread( const char* pThreadName = nullptr );
    ~TestDriveThread();

    bool initialize( StartRoutine_t pThreadStartRoutine, void* lpParameter, const size_t threadStackSize, const uint32_t dwCreationFlags );
    void setName( const char* pThreadName );
    bool setPriority( const eThreadPriority threadPriority );

private:
    size_t      stackSize;

#if defined( OTDU_WIN32 )
    DWORD       threadID;
    HANDLE      hThread;
#elif defined( OTDU_MACOS )
    pthread_t   threadHandle;
#endif

    std::string name;
};
