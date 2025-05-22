#pragma once

#include "shared.h"

#ifdef OTDU_WIN32
static float GetTimeSeconds()
{
    DWORD DVar1 = GetTickCount();
    float fVar2 = ( float )DVar1;

    if ( DVar1 < 0 ) {
        fVar2 = fVar2 + ( float )4.294967e+09f;
    }

    return fVar2 * 0.001f;
}

static float GetTimePrecise()
{
    static LARGE_INTEGER gPerformanceCounterFrequency;
    static double gPerformanceCounterFrequencyInv = 0;

    if ( gPerformanceCounterFrequency.QuadPart == 0 ) {
        BOOL BVar1 = QueryPerformanceFrequency((LARGE_INTEGER *)&gPerformanceCounterFrequency);
        if ( BVar1 == FALSE ) {
            return 0.0f;
        }
        gPerformanceCounterFrequencyInv = 1.0 / ( double )gPerformanceCounterFrequency.QuadPart;
    }

    LARGE_INTEGER local_c;
    BOOL BVar1 = QueryPerformanceCounter( &local_c );
    return ( BVar1 == FALSE ) ? 0.0f : static_cast<float>( local_c.QuadPart * gPerformanceCounterFrequencyInv );
}
#elif defined( OTDU_MACOS )
static float GetTimeSeconds()
{
    struct timespec t;
    clock_gettime( CLOCK_MONOTONIC, &t );
    return t.tv_sec + (t.tv_nsec / 1000000);
}

static float GetTimePrecise()
{
    static double gPerformanceCounterFrequencyInv =  1.0 / 1000000000.0;

    struct timespec t;
    clock_gettime( CLOCK_THREAD_CPUTIME_ID, &t );
    return static_cast<float>( t.tv_nsec * gPerformanceCounterFrequencyInv );
}
#endif
