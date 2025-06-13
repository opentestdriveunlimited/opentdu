#pragma once

class TestDriveEvent {
public:
    TestDriveEvent( const char* pEventName = nullptr );
    ~TestDriveEvent();

    void destroy();
    void initialize( const char* pEventName );
    bool waitForEvent();

private:
#if defined( OTDU_WIN32 )
    HANDLE          semaphore;
#elif defined( OTDU_MACOS )
    // TODO: macOS doesnt support POSIX unamed semaphore (special snowflake OS)
    // Using spinlock to emulate semaphore...
    std::atomic<bool> semaphore;
#endif

    std::string     name;
    uint8_t         bInitialized : 1;
    uint8_t         bMarkedForDeletion : 1;
};
