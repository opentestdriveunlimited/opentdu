#pragma once

static uint32_t gCRC32LUT[256] = { 0u };

static void TestDriveCRC32InitLUT()
{
    uint32_t uVar1 = 0;
    uint32_t uVar2 = 0;

    do {
        uVar1 = uVar2 >> 1;
        if ( ( uVar2 & 1 ) != 0 ) {
            uVar1 = uVar1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        gCRC32LUT[uVar2] = uVar1;
        uVar2++;
    } while ( uVar2 < 256 );
}

static uint32_t TestDriveCRC32( const uint8_t* pContent, size_t contentLength )
{
    static bool bLUTInitialized = false;
    if ( !bLUTInitialized ) {
        TestDriveCRC32InitLUT();
        bLUTInitialized = true;
    }

    uint32_t uVar1 = 0xffffffff;
    if ( 0 < contentLength ) {
        do {
            uVar1 = uVar1 >> 8 ^ gCRC32LUT[( *pContent ^ uVar1 ) & 0xff];
            pContent = pContent + 1;
            contentLength = contentLength + -1;
        } while ( contentLength != 0 );
    }
    return ~uVar1 >> 0x10 ^ ~uVar1;
}

static uint32_t TestDriveCRC32( const char* pString )
{
    return TestDriveCRC32( ( const uint8_t* )pString, strlen( pString ) );
}

static uint32_t TestDriveCRC32( const std::string& string )
{
    return TestDriveCRC32( ( const uint8_t* )string.c_str(), string.length() );
}
