#pragma once

#include "logger.h"

extern bool gDisableAssert;
extern bool gBreakOnAssertFailure; // DAT_00f47931

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
if ( !gDisableAssert && !( condition ) ) {\
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
    OTDU_FATAL_ERROR( "Fatal: Assertion failure file: %s line: %u\n", __FILE__, __LINE__ );\
}

#define OTDU_FATAL_ERROR( fmt, ... )\
    OTDU_LOG_ERROR("Fatal: " fmt, ##__VA_ARGS__);\
    if ( gBreakOnAssertFailure ) {\
        OTDU_TRIGGER_BREAKPOINT;\
    }\
    OnFatalError();\
