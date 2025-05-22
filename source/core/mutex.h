#pragma once

class TestDriveMutex {
public:
    TestDriveMutex( const char* pMutexName = nullptr );
    ~TestDriveMutex();

    void initialize( const char* pMutexName = nullptr );
    void terminate();
    void lock();
    void unlock();

private:
    std::string name;

#if defined( OTDU_WIN32 )
    CRITICAL_SECTION criticalSection;
    DWORD lockedByThreadID;
#elif defined( OTDU_MACOS )
    pthread_mutex_t mutex;
#endif

    uint32_t lockCount;
    float totalLockDurationInSeconds;
    float lockStartTimeInSeconds;
    uint8_t bLocked : 1;
    uint8_t bInitialized : 1;
    uint8_t bMarkedForDeletion : 1;
};