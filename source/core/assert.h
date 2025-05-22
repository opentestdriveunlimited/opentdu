#pragma once

#include "logger.h"

static bool gSkipAssert = false;
static bool gBreakOnAssertFailure = true;

[[noreturn]] static void OnFatalError()
{
    abort();
}

#if OTDU_MSVC
#define OTDU_TRIGGER_BREAKPOINT __debugbreak();
#elif OTDU_GCC
#include <signal.h>
#define OTDU_TRIGGER_BREAKPOINT raise(SIGTRAP);
#elif OTDU_CLANG
#define OTDU_TRIGGER_BREAKPOINT __builtin_debugtrap();
#else
#define OTDU_TRIGGER_BREAKPOINT
#endif

#ifdef OTDU_DEVBUILD
#define OTDU_ASSERT( condition )\
if ( !gSkipAssert && !( condition ) ) {\
    OTDU_LOG_ERROR( "Assertion failure file: %s line: %u\n", __FILE__, __LINE__ );\
    if ( gBreakOnAssertFailure ) {\
        OTDU_TRIGGER_BREAKPOINT;\
    }\
}
#else
#define OTDU_ASSERT( condition )
#endif

#define OTDU_ASSERT_FATAL( condition )\
if ( !( condition ) ) {\
    if ( gBreakOnAssertFailure ) {\
        OTDU_TRIGGER_BREAKPOINT;\
        OnFatalError();\
    }\
}
