#pragma once

#include <stdint.h>
#include <string>
#include <array>
#include <fstream>
#include <functional>
#include <algorithm>

#include <Eigen/Dense>

extern const char** gpCmdLineArgs;
extern int32_t gCmdLineArgCount;

#include "core/assert.h"

#ifdef OTDU_DEVBUILD
#define OTDU_UNIMPLEMENTED OTDU_ASSERT(false);
#else
#define OTDU_UNIMPLEMENTED
#endif

static constexpr bool bUseCRC64 = false; // Matches original executable behavior

#include "core/hash/crc64.h"
#include "core/hash/tdu_concat.h"
#include "system/system_shared.h"

inline uint64_t GetIdentifier64bit( const char* pString )
{
    return ( bUseCRC64 ) 
        ? TestDriveCRC64( pString )
        : TestDriveConcatToString8( pString );
}

template<typename T>
inline T Min( T left, T right )
{
    return left < right ? left : right;
}

template<typename T, typename R>
inline T Min( T left, R right )
{
    return left < (T)right ? left : (T)right;
}

template<typename T>
inline T Max( T left, T right )
{
    return left < right ? right : left;
}

template<typename T, typename R>
inline T Max( T left, R right )
{
    return left < (T)right ? (T)right : left;
}

template<typename T>
inline T Clamp( T value, T minValue, T maxValue )
{
    return Max(Min(value, maxValue), minValue);
}
