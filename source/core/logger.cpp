#include "shared.h"
#include "logger.h"

#include "assert.h"

#include <cstdarg>
#include <iostream>

#if OTDU_MSVC
inline void OutputToDebugger( const char* pFormatedString )
{
    OutputDebugString( pFormatedString );
}
#else
inline void OutputToDebugger( const char* pFormatedString )
{
#ifndef OTDU_LOG_STDOUT // Redundant if we already redirect logs to stdout...
    printf( "%s", pFormatedString );
#endif
}
#endif

static constexpr int32_t MaxLogSize = 8192;

void GlobalLogger::Write( const char* pFormat, ... )
{
    thread_local char pBuffer[MaxLogSize];

    va_list argList;
    va_start( argList, pFormat );

    int32_t bufferOutputSize = vsnprintf( pBuffer, sizeof( pBuffer ), pFormat, argList );
    OTDU_ASSERT( bufferOutputSize < sizeof( pBuffer ) );

    va_end( argList );

    // Bad format/buffer overrun; early exit to avoid hard crash.
    if ( bufferOutputSize == -1 ) {
        OTDU_TRIGGER_BREAKPOINT;
        return;
    }

#ifdef OTDU_LOG_STDOUT
    printf( "%s", pBuffer );
#endif

    OutputToDebugger( pBuffer );
}
