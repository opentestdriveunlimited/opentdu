#pragma once

class GlobalLogger {
public:
    static void Write( const char* pFormat, ... );
};

#if OTDU_MSVC
#define OTDU_STRINGIFY_EXPAND( x ) #x
#define OTDU_STRINGIFY( x ) OTDU_STRINGIFY_EXPAND( x )
#else
#define OTDU_STRINGIFY( x )
#endif

#ifdef OTDU_DEVBUILD
#define OTDU_LOG_INFO( format, ... ) GlobalLogger::Write( "[LOG]\t" __FILE__  ":" OTDU_STRINGIFY( __LINE__ ) " >> " format, ##__VA_ARGS__ )
#define OTDU_LOG_WARN( format, ... ) GlobalLogger::Write( "[WARN]\t" __FILE__  ":" OTDU_STRINGIFY( __LINE__ ) " >> " format, ##__VA_ARGS__ )
#define OTDU_LOG_ERROR( format, ... ) GlobalLogger::Write( "[ERROR]\t" __FILE__  ":" OTDU_STRINGIFY( __LINE__ ) " >> " format, ##__VA_ARGS__ )
#define OTDU_LOG_DEBUG( format, ... ) GlobalLogger::Write( "[DEBUG]\t" __FILE__ ":" OTDU_STRINGIFY( __LINE__ ) " >> " format, ##__VA_ARGS__ )
#else
#define OTDU_LOG_INFO( format, ... )
#define OTDU_LOG_WARN( format, ... )
#define OTDU_LOG_ERROR( format, ... )
#define OTDU_LOG_DEBUG( format, ... )
#endif

#define OTDU_LOG_ALWAYS( format, ... ) GlobalLogger::Write( format, ##__VA_ARGS__ )