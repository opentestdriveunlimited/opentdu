#pragma once

// 8bytes concat string (mainly used by GSRender/geometry assets)
struct TestDriveString8 
{
    union {
        char        AsChar[8];
        uint64_t    AsInteger64;
    };

    TestDriveString8& operator = (const uint64_t integer)
    {
        AsInteger64 = integer;
        return *this;
    }

    TestDriveString8& operator = (const char* pString)
    {
        OTDU_ASSERT(strlen(pString) <= 8);
        strncpy(AsChar, pString, strlen(pString));
        return *this;
    }

    inline char& operator [] ( const uint32_t idx )
    {
        return AsChar[idx];
    }

    operator uint64_t()
    {
        return AsInteger64;
    }
};

static TestDriveString8 TestDriveConcatToString8( const char* pContent, size_t contentLength )
{
    // TODO: Did they implement an already existing algorithm; or is it something custom?
    TestDriveString8 output;
    output.AsInteger64 = 0ull;

    for ( size_t i = 0ull; i < contentLength; i++ ) {
        char readChar = pContent[i];
        if ( ( '`' < readChar ) && ( readChar < '{' ) ) {
            readChar += -0x20;
        }

        const uint32_t storageIdx = static_cast<uint32_t>( i & 7 );
        output[storageIdx] += readChar;
    }

    return output;
}

static TestDriveString8 TestDriveConcatToString8( const char* pString )
{
    return TestDriveConcatToString8( pString, strlen( pString ) );
}

static TestDriveString8 TestDriveConcatToString8( const std::string& string )
{
    return TestDriveConcatToString8( string.c_str(), string.length() );
}
