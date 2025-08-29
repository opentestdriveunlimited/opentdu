#pragma once

#include <stdint.h>
#include <string>
#include <array>
#include <fstream>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <stack>
#include <queue>
#include <list>
#include <thread>
#include <set>
#include <locale>
#include <codecvt>
#include <stdio.h>
#include <time.h>

#include <Eigen/Dense>

extern const char** gpCmdLineArgs;
extern int32_t gCmdLineArgCount;

#include "core/assert.h"

#ifdef OTDU_DEVBUILD
// Function needs to be implemented
#define OTDU_UNIMPLEMENTED OTDU_ASSERT(false);

// Function is not implemented (and is probably not required)
#define OTDU_IMPLEMENTATION_SKIPPED( funAddress )
#else
#define OTDU_UNIMPLEMENTED
#define OTDU_IMPLEMENTATION_SKIPPED( funAddress )
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

static const Eigen::Vector3f kWorldUpVector(0.0f, 1.0f, 0.0f);

static std::string IntegerToHexString( const size_t w, const size_t hex_len = sizeof( size_t ) << 1 )
{
    static constexpr const char* kDigits = "0123456789ABCDEF";
    std::string rc( hex_len, '0' );
    for ( size_t i = 0, j = ( hex_len - 1 ) * 4; i < hex_len; ++i, j -= 4 )
        rc[i] = kDigits[( w >> j ) & 0x0f];
    return rc;
}

inline void ThreadYield()
{
    std::this_thread::yield();
}

#ifndef OTDU_MSVC
static char* strlwr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = tolower((unsigned char) *tmp);
    }

    return s;
}

static char* strupr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = toupper((unsigned char) *tmp);
    }

    return s;
}
#endif

#if defined(__x86_64__) || defined(_WIN64) || defined(__aarch64__)
#define OTDU_64_BITS 1
#define OTDU_32_BITS 0
#else
#define OTDU_64_BITS 0
#define OTDU_32_BITS 1
#endif

#include "x86_pointer_emulation.h"

// Helper for generic static assert checking the size of the object. Useful for
// any class/struct used by assets
#define OTDU_SIZE_MUST_MATCH( obj, size ) static_assert( sizeof( obj ) == size, #obj ": size does not match (asset deserialization will fail!)")
