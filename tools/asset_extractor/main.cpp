#include "shared.h"

#include "core/arg_parser.h"
#include "core/logger.h"
#include "core/assert.h"

#include "render/shaders/shader_table_master.h"

#include <fstream>
#include <filesystem>

//#define EXCLUDE_PSTDINT
//#include "hlslcc.h"

static char gpPathToExecutable[OTDU_MAX_PATH]; // Path to TestDriveUnlimited.exe
static CmdLineArg CmdLineArgsExecutablePath( "executable", []( const char* pArg ) {
    strcpy( gpPathToExecutable, pArg );
} );

static char gpOutputPath[OTDU_MAX_PATH]; // (Base) Output path for asset extraction
static CmdLineArg CmdLineArgsOutputPath( "asset_output_path", []( const char* pArg ) {
    strcpy( gpOutputPath, pArg );
} );

static constexpr uint32_t kD3D9ShaderMagic  = 0xffff0200; // 0x0002ff;
static constexpr uint32_t kD3D9ShaderMagic2 = 0xfffe0200; // 0x0002ff;

static constexpr uint32_t kD3D9ShaderMagic3 = 0x42415443;

struct TestDrivePackedIniEntry {
    uint32_t FirstDword = 0u;
    uint32_t LengthInBytes = 0u;
    const char* Name = nullptr;
};

static constexpr TestDrivePackedIniEntry kSystemPCIni = { 0x0A2A085B, 0x357, "SystemPC.ini" }; // 0x5B082A0A
static constexpr TestDrivePackedIniEntry kGamePCIni = { 0x1A3D095B, 0x14ab, "GamePC.ini" }; // 0x5B093D1A
static constexpr TestDrivePackedIniEntry kDevicesPCIni = { 0x1136085B, 0x1545, "DevicesPC.ini" }; // 0x5B083611
static constexpr TestDrivePackedIniEntry kAudioIni = { 0x0707070D, 0x1265, "Audio.ini" }; // 0x0d070707
static constexpr TestDrivePackedIniEntry kReplayIni = { 0x1537095B, 0x407, "Replay.ini" }; // 0x5B093715
static constexpr TestDrivePackedIniEntry kPhysicsIni = { 0x1322185B, 0xdfe, "Physics.ini" }; // 0x5B182213

static constexpr uint32_t kPackedIniFileCount = 6;
static constexpr TestDrivePackedIniEntry kPackedIniFiles[kPackedIniFileCount] = {
    kSystemPCIni,
    kGamePCIni,
    kDevicesPCIni,
    kAudioIni,
    kReplayIni,
    kPhysicsIni
};

// Unpack .ini (in place).
static void UnpackIniFromMemory( int8_t* pBuffer, const uint32_t bufferSizeInBytes ) {
    int8_t* pBufferStart = pBuffer;
    int8_t* pBufferEnd = pBuffer + bufferSizeInBytes;

    int8_t currentByte = 0x0;
    while ( pBufferStart != pBufferEnd ) {
        int8_t readByte = *pBufferStart;
        readByte ^= currentByte;
        *pBufferStart = readByte;
        pBufferStart++;
        currentByte = readByte;
    }
}

static int32_t Align( const int32_t value, const int32_t alignment ) {
    const int32_t mask = alignment - 1;
    return value + ( -value & mask );
}

static std::string IntegerToHexString( const size_t w, const size_t hex_len = sizeof( size_t ) << 1 ) {
    static constexpr const char* kDigits = "0123456789ABCDEF";
    std::string rc( hex_len, '0' );
    for ( size_t i = 0, j = ( hex_len - 1 ) * 4; i < hex_len; ++i, j -= 4 )
        rc[i] = kDigits[( w >> j ) & 0x0f];
    return rc;
}

static void RealignReadPointer( std::ifstream& fileStream, const int32_t readByteCount ) {
    int32_t alignedReadByteCount = Align( readByteCount, sizeof( uint32_t ) );
    int32_t byteCountToSkip = alignedReadByteCount - readByteCount;
    fileStream.seekg( byteCountToSkip, std::ios_base::cur );
}

static std::string GetIniOutputPath() {
    return std::string( gpOutputPath ) + "/";
}

static std::string GetShaderOutputPath() {
    return std::string( gpOutputPath ) + "/shaders/";
}

static void CheckAndCreateOutputFolder( const std::string& folder ) {
    std::filesystem::path folderFS = std::filesystem::path( folder );
    if ( !std::filesystem::exists( folderFS ) ) {
        OTDU_LOG_DEBUG( "'%s' does not exist yet; creating directory...\n", folder.c_str() );
        std::filesystem::create_directories( folderFS );
    }
}

[[noreturn]] static void PrintUsage() {
    OTDU_LOG_ALWAYS( "USAGE: opentdu_assetExtractor -executable <pathToExe> [-asset_output_path <pathToOutput>]\n"
                     "\n"
                     "-executable:            Path to your TestDriveUnlimited.exe executable\n"
                     "-asset_output_path:     Path used to write extracted assets (optional) (default: current working directory)\n" );
    exit( 1 );
}

int main( int argc, char* argv[] ) {
    // Parse cmdline args
    memset( gpPathToExecutable, 0, sizeof( char ) * OTDU_MAX_PATH );
    memset( gpOutputPath, 0, sizeof( char ) * OTDU_MAX_PATH );

    gArgParserRegister.parseCmdLineArgs( (const char**)argv, argc );

    if ( *gpPathToExecutable == '\0' ) {
        OTDU_LOG_ALWAYS( "Missing cmdline arg 'executable'!\n" );
        PrintUsage();
    }

    if ( *gpOutputPath == '\0' ) {
        TestDrive::GetWorkingDir( gpOutputPath, OTDU_MAX_PATH );
        OTDU_LOG_ALWAYS( "No output path provided ('asset_output_path' cmdline arg not found); will use current working directory\n" );
    }

    OTDU_LOG_DEBUG( "executable: '%s'\n", gpPathToExecutable );
    OTDU_LOG_DEBUG( "asset_output_path: '%s'\n", gpOutputPath );

    // Prepare output
    CheckAndCreateOutputFolder( GetIniOutputPath() );
    CheckAndCreateOutputFolder( GetShaderOutputPath() );

    uint32_t shaderExtractedCount = 0;
    uint32_t iniExtractedCount = 0;

    std::ifstream fileStream;
    fileStream.open( gpPathToExecutable, std::ios_base::binary | std::ios_base::in );
    if ( !fileStream.is_open() || !fileStream.good() ) {
        OTDU_LOG_ALWAYS( "Failed to open executable '%s' for read!\n", gpPathToExecutable );
        return 1;
    }

    constexpr uint32_t kNumShaderEntries = sizeof( kMasterShaderTable ) / sizeof( kMasterShaderTable[0] );
    uint32_t foundShaderEntries = 0u;

    int32_t dword = 0;
    for ( uint32_t i = 0; i < kNumShaderEntries; i++ ) {
        const ShaderTableEntry& entry = kMasterShaderTable[i];

        size_t offsetToSeek = entry.OffsetInExecutable - 0x400000;
        fileStream.seekg( offsetToSeek, std::ios::beg);

        std::vector<int8_t> shaderBytecode;
        shaderBytecode.reserve( 2 << 10 ); // Should be large enough (biggest shaders are ~2KiB) 
        shaderBytecode.resize( 0x18 );

        fileStream.read( reinterpret_cast< char* >( shaderBytecode.data() ), 0x18 * sizeof( int8_t ) );

        if (    shaderBytecode[8]  == 'C'
                && shaderBytecode[9]  == 'T'
                && shaderBytecode[10] == 'A'
                && shaderBytecode[11] == 'B' ) {
            OTDU_LOG_INFO( "Reading shader binary at offset %x\n", entry.OffsetInExecutable );

            int16_t word = 0;
            constexpr int16_t kEndShaderMarker = 0xffff;
            while ( word != kEndShaderMarker ) {
                fileStream.read( reinterpret_cast< char* >( &word ), sizeof( int16_t ) );

                shaderBytecode.push_back( word & 0xff );
                shaderBytecode.push_back( ( word >> 8 ) & 0xff );
            }
        } else {
            OTDU_LOG_INFO( "INVALID MAGIC FOUND\n");
        }

        // Write to disk
        std::string filename = IntegerToHexString( entry.OffsetInExecutable ) + ".dxso";
        std::string fileOutputPath = GetShaderOutputPath() + filename;

        std::filesystem::path folderFS = std::filesystem::path( fileOutputPath );
        if ( !std::filesystem::exists( folderFS ) ) {
            /* GLSLShader shader = { 0 };
                TranslateHLSLFromMem( ( const char* )shaderBytecode.data(), 0x0, GLLang::LANG_440, nullptr, nullptr, &shader );

                OTDU_ASSERT( shader.sourceCode );*/

            std::ofstream outputFileStream;
            outputFileStream.open( fileOutputPath.c_str(), std::ios_base::out | std::ios_base::binary );
            outputFileStream.write( ( char* )shaderBytecode.data(), sizeof( char )* shaderBytecode.size() );
            outputFileStream.close();

            OTDU_LOG_INFO( "Wrote shader to '%s' (category '%s')\n", fileOutputPath.c_str(), entry.pShaderCategory );
            shaderExtractedCount++;
        } else {
            OTDU_LOG_INFO( "'%s' already exists; skipping...\n", fileOutputPath.c_str() );
        }
    }

    fileStream.seekg(0, std::ios::beg);
    dword = 0;

    while ( true ) {
        size_t shaderOffset = fileStream.tellg();
        if ( !fileStream.read( reinterpret_cast< char* >( &dword ), sizeof( int32_t ) ) ) {
            break;
        }

        // if ( dword == kD3D9ShaderMagic3 ) { // kD3D9ShaderMagic || dword == kD3D9ShaderMagic2 ) {
        //     shaderOffset -= 8;
        //     fileStream.seekg( shaderOffset, std::ios_base::beg );

        //     // Read header and make sure we find the constant table magic (CTAB) before extraction
        //     std::vector<int8_t> shaderBytecode;
        //     shaderBytecode.reserve( 2 << 10 ); // Should be large enough (biggest shaders are ~2KiB) 
        //     shaderBytecode.resize( 0x18 );

        //     fileStream.read( reinterpret_cast< char* >( shaderBytecode.data() ), 0x18 * sizeof( int8_t ) );

        //     shaderOffset += 0x400000; // Offset to map executable bytes to .rdata block offset

        //     if (    shaderBytecode[8]  == 'C'
        //          && shaderBytecode[9]  == 'T'
        //          && shaderBytecode[10] == 'A'
        //          && shaderBytecode[11] == 'B' ) {
        //         OTDU_LOG_INFO( "Reading shader binary at offset %x\n", shaderOffset );

        //         int16_t word = 0;
        //         constexpr int16_t kEndShaderMarker = 0xffff;
        //         while ( word != kEndShaderMarker ) {
        //             fileStream.read( reinterpret_cast< char* >( &word ), sizeof( int16_t ) );

        //             shaderBytecode.push_back( word & 0xff );
        //             shaderBytecode.push_back( ( word >> 8 ) & 0xff );
        //         }
        //     }

        //     // Write to disk
        //     std::string filename = IntegerToHexString( shaderOffset ) + ".dxso";
        //     std::string fileOutputPath = GetShaderOutputPath() + filename;

        //     std::filesystem::path folderFS = std::filesystem::path( fileOutputPath );
        //     if ( !std::filesystem::exists( folderFS ) ) {
        //         /* GLSLShader shader = { 0 };
        //          TranslateHLSLFromMem( ( const char* )shaderBytecode.data(), 0x0, GLLang::LANG_440, nullptr, nullptr, &shader );

        //          OTDU_ASSERT( shader.sourceCode );*/

        //         std::ofstream outputFileStream;
        //         outputFileStream.open( fileOutputPath.c_str(), std::ios_base::out | std::ios_base::binary );
        //         outputFileStream.write( ( char* )shaderBytecode.data(), sizeof( char )* shaderBytecode.size() );
        //         outputFileStream.close();

        //         OTDU_LOG_INFO( "Wrote shader to '%s'\n", fileOutputPath.c_str() );
        //         shaderExtractedCount++;
        //     } else {
        //         OTDU_LOG_INFO( "'%s' already exists; skipping...\n", fileOutputPath.c_str() );
        //     }

        //     // Realign stream pointer
        //     RealignReadPointer( fileStream, shaderBytecode.size() );
        // } else {
            // .ini files
            for ( uint32_t i = 0; i < kPackedIniFileCount; i++ ) {
                const TestDrivePackedIniEntry& packedIni = kPackedIniFiles[i];
                if ( dword == packedIni.FirstDword ) {
                    OTDU_LOG_INFO( "Reading packed config file '%s'...\n", packedIni.Name );

                    int8_t* pBuffer = new int8_t[packedIni.LengthInBytes];
                    memset( pBuffer, 0, sizeof( int8_t ) * packedIni.LengthInBytes );
                    *( int32_t* )pBuffer = dword;

                    fileStream.read( ( char* )( pBuffer + sizeof( int32_t ) ), 
                                     sizeof( int8_t ) * ( packedIni.LengthInBytes - sizeof( int32_t ) ) );
                    UnpackIniFromMemory( pBuffer, packedIni.LengthInBytes );

                    // Write to disk
                    std::string fileOutputPath = GetIniOutputPath() + packedIni.Name;

                    std::ofstream outputFileStream;
                    outputFileStream.open( fileOutputPath.c_str(), std::ios_base::out );
                    outputFileStream.write( ( char* )pBuffer, sizeof( char ) * packedIni.LengthInBytes );
                    outputFileStream.close();

                    OTDU_LOG_INFO( "Wrote .ini to '%s'\n", fileOutputPath.c_str() );

                    delete[] pBuffer;

                    iniExtractedCount++;

                    // Realign stream pointer
                    RealignReadPointer( fileStream, packedIni.LengthInBytes );
                    break;
                }
            }
        //}
    }
    fileStream.close();

    OTDU_LOG_ALWAYS( "==== Summary ====\nFound and extracted %u shaders and %u .ini files to disk\n",
                    shaderExtractedCount, iniExtractedCount );

    return 0;
}
