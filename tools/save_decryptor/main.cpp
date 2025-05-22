#include "shared.h"

#include "core/arg_parser.h"
#include "core/logger.h"
#include "core/assert.h"
#include "core/crypto/tea.h"

#include <fstream>
#include <filesystem>

static char gpPathToCommondt[OTDU_MAX_PATH]; // Path to commondt.sav
static CmdLineArg CmdLineArgsCommondtPath( "commondt_path", []( const char* pArg ) {
    strcpy( gpPathToCommondt, pArg );
} );

[[noreturn]] static void PrintUsage() {
    OTDU_LOG_ALWAYS( "USAGE: opentdu_savedecryptor -commondt_path <pathToCommondt>\n"
                     "\n"
                     "-commondt_path:            Path to your commondt.sav savegame file\n" );
    exit( 1 );
}

int main( int argc, char* argv[] ) {
    // Parse cmdline args
    memset( gpPathToCommondt, 0, sizeof( char ) * OTDU_MAX_PATH );

    gArgParserRegister.parseCmdLineArgs( (const char**)argv, argc );

    if ( *gpPathToCommondt == '\0' ) {
        OTDU_LOG_ALWAYS( "Missing cmdline arg 'commondt_path'!\n" );
        PrintUsage();
    }

    OTDU_LOG_DEBUG( "commondt_path: '%s'\n", gpPathToCommondt );

    std::ifstream fileStream;
    fileStream.open( gpPathToCommondt, std::ios_base::binary | std::ios_base::in );
    if ( !fileStream.is_open() || !fileStream.good() ) {
        OTDU_LOG_ALWAYS( "Failed to open save configuration '%s' for read!\n", gpPathToCommondt );
        return 1;
    }
    
    std::ofstream decryptedFileStream;
    decryptedFileStream.open( "D:/playersave_out.sav", std::ios_base::binary | std::ios_base::out );
    while ( fileStream.good() ) {
        uint32_t encryptedDwords[2];
        fileStream.read( ( char* )encryptedDwords, sizeof( uint32_t ) * 2 );

        uint32_t decryptedDwords[2];
        TestDriveTEADecrypt( decryptedDwords, encryptedDwords );

        decryptedFileStream.write( ( char* )decryptedDwords, sizeof( uint32_t ) * 2 );
    }

    fileStream.close();

    return 0;
}
