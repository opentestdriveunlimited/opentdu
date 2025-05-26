#include "shared.h"
#include "arg_parser.h"

#include "logger.h"
#include "assert.h"

ArgParserRegister gpArgParserRegister = {};

ArgParserRegister::ArgParserRegister()
    : registeredParserCount( 0 )
{

}

ArgParserRegister::~ArgParserRegister()
{

}

ArgParserRegister::ParserHandle_t ArgParserRegister::registerParser( const char* pArgValue, ArgParserRegister::ParserCallback_t& pParser )
{
    OTDU_ASSERT_FATAL( registeredParserCount < 32 );

    ParserHandle_t parserHandle = registeredParserCount;

    ArgParserEntry& parser = registeredParsers[registeredParserCount];
    parser.Arg = std::string( pArgValue );
    parser.pParser = pParser;

    registeredParserCount++;
    
    return parserHandle;
}

void ArgParserRegister::parseCmdLineArgs( const char** pCmdLineArgs, const int32_t cmdLineArgCount )
{
    if ( pCmdLineArgs == nullptr ) {
        return;
    }

    for ( int32_t argIndex = 0; argIndex < cmdLineArgCount; argIndex++ ) {
        const char* pArg = pCmdLineArgs[argIndex];
        if ( *pArg == '-' ) {
            const char* pArgValue = ++pArg;

            const ArgParserEntry* pArgParser = findArgParser( pArgValue );
            if ( pArgParser != nullptr ) {
                do {
                    ++pArgValue;
                } while ( *pArgValue != ' ' && *pArgValue != '\0' );

                pArgParser->pParser( pArgValue + 1 );
            } else {
                OTDU_LOG_WARN( "Unknown cmdline argument '%s'\n", pArgValue );
            }
        }
    }
}

const ArgParserRegister::ArgParserEntry* ArgParserRegister::findArgParser( const char* pArgValue ) const
{
    if ( registeredParserCount == 0 ) {
        return nullptr;
    }

    std::string argValueString( pArgValue );
    for ( uint32_t parserIdx = 0; parserIdx < registeredParserCount; parserIdx++ ) {
        const ArgParserEntry& parser = registeredParsers[parserIdx];
        if ( parser.Arg == argValueString ) {
            return &parser;
        }
    }

    return nullptr;
}
