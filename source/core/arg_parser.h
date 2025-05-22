#pragma once

#include "shared.h"

class ArgParserRegister 
{
public:
    using ParserCallback_t = std::function<void( const char* )>;
    using ParserHandle_t = uint32_t;

public:
    ArgParserRegister();
    ~ArgParserRegister();

    ParserHandle_t  registerParser( const char* pArgValue, ParserCallback_t& pParser );
    void            parseCmdLineArgs( const char** pCmdLineArgs, const int32_t cmdLineArgCount );

private:
    struct ArgParserEntry {
        ParserCallback_t    pParser;
        std::string         Arg;
    };

private:
    const ArgParserEntry* findArgParser( const char* pArgValue ) const;

private:
    std::array<ArgParserEntry, 32>    registeredParsers;
    uint32_t                          registeredParserCount;
};

static ArgParserRegister gArgParserRegister;

struct CmdLineArg {
    CmdLineArg( const char* pArgValue, ArgParserRegister::ParserCallback_t pParsingFunc ) {
        handle = gArgParserRegister.registerParser( pArgValue, pParsingFunc );
    }

    ArgParserRegister::ParserHandle_t handle;
};

static bool ParseArgValueFromCmdLine( const char* pArgValue, char* pParsedValue, const size_t parseValueMaxLength )
{
    for ( int32_t i = 0; i < gCmdLineArgCount; i++ ) {
        const char* pCmdLineArg = gpCmdLineArgs[i];
        if ( *pCmdLineArg == '-' ) {
#if defined( OTDU_WIN32 )
            int32_t comparisonResult = _stricmp( pArgValue, pCmdLineArg + 1 );
#else
            int32_t comparisonResult = strcasecmp( pArgValue, pCmdLineArg + 1 );
#endif
            if ( comparisonResult == 0 && i < ( gCmdLineArgCount + -1 ) ) {
                const char* pCmdLineValue = gpCmdLineArgs[i + 1];

                strncpy( pParsedValue, pCmdLineValue, strlen( pCmdLineValue ) );
                return true;
            }
        }
    }

    return false;
}
