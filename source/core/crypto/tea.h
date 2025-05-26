#pragma once

static uint32_t gTEAk[4] = { 0x64EA432C,    0xF8A35B24,     0x18ECD81,    0x8326BEAC };

static void TestDriveTEADecrypt( uint32_t* pOutV, const uint32_t* pInV, uint32_t k[4] )
{
    uint32_t sum = 0xc6ef3720;

    uint32_t v0 = pInV[0];
    uint32_t v1 = pInV[1];

    for ( int32_t i = 0; i < 32; i++ ) {
        v1 = v1 - ( ( v0 >> 5 ^ v0 << 4 ) + v0 ^ k[sum >> 0xb & 3] + sum );
        sum += 0x61c88647;
        v0 = v0 - ( ( v1 >> 5 ^ v1 * 0x10 ) + v1 ^ k[sum & 3] + sum );
    }

    pOutV[0] = v0;
    pOutV[1] = v1;
}

static void TestDriveTEAEncrypt( uint32_t* pOutV, const uint32_t* pInV, uint32_t k[4] )
{
    uint32_t sum = 0;

    uint32_t v0 = pInV[0];
    uint32_t v1 = pInV[1];

    for ( int32_t i = 0; i < 32; i++ ) {
        v0 = v0 + ( ( v1 >> 5 ^ v1 << 4 ) + v1 ^ k[sum & 3] + sum );
        sum += 0x9e3779b9;
        v1 = v1 + ( ( v0 >> 5 ^ v0 * 0x10 ) + v0 ^ k[sum >> 0xb & 3] + sum );
    }

    pOutV[0] = v0;
    pOutV[1] = v1;
}

inline void TestDriveTEADecrypt( uint32_t* pOutV, const uint32_t* pInV )
{
    TestDriveTEADecrypt( pOutV, pInV, gTEAk );
}

inline void TestDriveTEAEncrypt( uint32_t* pOutV, const uint32_t* pInV )
{
    TestDriveTEAEncrypt( pOutV, pInV, gTEAk );
}
