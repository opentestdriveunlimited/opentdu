#include "shared.h"

#include "core/arg_parser.h"
#include "core/logger.h"
#include "core/assert.h"

#include "render/shaders/shader_register.h"
#include "render/shaders/shader_table_master.h"

#include <fstream>
#include <filesystem>

#include "dxvk/src/dxso/dxso_module.h"
#include "dxvk/src/dxso/dxso_compiler.h"
#include "dxvk/src/dxso/dxso_reader.h"
#include "dxvk/src/dxso/dxso_analysis.h"
#include "dxvk/src/d3d9/d3d9_constant_layout.h"

#include "spirv_cross/spirv_glsl.hpp"

static char gpPathToExecutable[OTDU_MAX_PATH]; // Path to TestDriveUnlimited.exe
static CmdLineArg CmdLineArgsExecutablePath( "executable", []( const char* pArg ) {
    strcpy( gpPathToExecutable, pArg );
} );

static char gpOutputPath[OTDU_MAX_PATH]; // (Base) Output path for asset extraction
static CmdLineArg CmdLineArgsOutputPath( "asset_output_path", []( const char* pArg ) {
    strcpy( gpOutputPath, pArg );
} );

static bool gbSkipGLSLTranslation = false; // If true, do not generate shader tables for OpenGL
static CmdLineArg CmdLineArgsSkipGLSL( "skip_glsl", []( const char* pArg ) {
    gbSkipGLSLTranslation = true;
} );

static bool gbForceExtraction = false; // If true, ignore previously extracted assets
static CmdLineArg CmdLineArgsForce( "force", []( const char* pArg ) {
    gbForceExtraction = true;
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

static void RealignReadPointer( std::ifstream& fileStream, const int32_t readByteCount ) {
    int32_t alignedReadByteCount = Align( readByteCount, sizeof( uint32_t ) );
    int32_t byteCountToSkip = alignedReadByteCount - readByteCount;
    fileStream.seekg( byteCountToSkip, std::ios_base::cur );
}

static std::string GetIniOutputPath() {
    return std::string( gpOutputPath ) + "/";
}

static std::string GetShaderOutputPath() {
    return std::string( gpOutputPath ) + "/" + kShadersRootFolder;
}

static void CheckAndCreateOutputFolder( const std::string& folder ) {
    std::filesystem::path folderFS = std::filesystem::path( folder );
    if ( !std::filesystem::exists( folderFS ) ) {
        OTDU_LOG_DEBUG( "'%s' does not exist yet; creating directory...\n", folder.c_str() );
        std::filesystem::create_directories( folderFS );
    }
}

[[noreturn]] static void PrintUsage() {
    OTDU_LOG_ALWAYS( "USAGE: opentdu_assetExtractor [-executable <pathToExe> -asset_output_path <pathToOutput> -skip_glsl -force]\n"
                     "\n"
                     "-executable:            Path to your TestDriveUnlimited.exe executable (optional) (default: executable in current working directory)\n"
                     "-asset_output_path:     Path used to write extracted assets (optional) (default: current working directory)\n"
                     "-skip_glsl:             Do not generate shader tables for OpenGL3.3 (optional) (default: false)\n"
                     "-force:                 Force asset extraction and ignore previously extracted assets (optional) (default: false)\n" );
    exit( 1 );
}

static void WriteShaderTableHeaderToStream(const ShaderTableHeader& header, std::ofstream& outputFileStream)
{
    size_t tableSize = header.GetHeaderSize();
    outputFileStream.write( ( const char* )&header.NumEntries, sizeof( size_t ) );
    for (auto& headerEntry : header.Entries) {
        uint64_t hashcode = std::get<0>(headerEntry);
        uint64_t offsetInFile = std::get<1>(headerEntry);
        offsetInFile += tableSize;

        outputFileStream.write( ( const char* )&hashcode, sizeof( uint64_t ) );
        outputFileStream.write( ( const char* )&offsetInFile, sizeof( uint64_t ) );
    }
}

static bool WriteShaderTableToDisk( const ShaderTable& shaderTable, const std::string& fileOutputPath)
{
    std::filesystem::path folderFS = std::filesystem::path( fileOutputPath );
    if ( !gbForceExtraction && std::filesystem::exists( folderFS ) ) {
        OTDU_LOG_INFO( "'%s' already exists; skipping...\n", fileOutputPath.c_str() );
        return false;
    }

    std::ofstream outputFileStream;
    outputFileStream.open( fileOutputPath.c_str(), std::ios_base::out | std::ios_base::binary );

    WriteShaderTableHeaderToStream( shaderTable.Header, outputFileStream );
    outputFileStream.write( ( const char* )shaderTable.Shaders.data(), sizeof( char ) * shaderTable.Shaders.size() );
    outputFileStream.close();

    OTDU_LOG_INFO( "Wrote shader table '%s'\n", fileOutputPath.c_str() );
    return true;
}

int main( int argc, char* argv[] ) {
    // Parse cmdline args
    memset( gpPathToExecutable, 0, sizeof( char ) * OTDU_MAX_PATH );
    memset( gpOutputPath, 0, sizeof( char ) * OTDU_MAX_PATH );

    gArgParserRegister.parseCmdLineArgs( (const char**)argv, argc );

    if ( *gpPathToExecutable == '\0' ) {
        std::filesystem::path folderFS = std::filesystem::path( "TestDriveUnlimited.exe" );
        if ( std::filesystem::exists( folderFS ) ) {
            strcpy(gpPathToExecutable, "TestDriveUnlimited.exe");
            OTDU_LOG_ALWAYS( "No executable path provided; will use executable in the current working directory\n" );
        } else {
            OTDU_LOG_ALWAYS( "Missing cmdline arg 'executable'!\n" );
            PrintUsage();
        }
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
    CheckAndCreateOutputFolder( GetShaderOutputPath() + kShadersD3D9Folder );
    CheckAndCreateOutputFolder( GetShaderOutputPath() + kShadersOpenGLFolder );
    CheckAndCreateOutputFolder( GetShaderOutputPath() + kShadersVulkanFolder );

    uint32_t shaderExtractedCount = 0;
    uint32_t iniExtractedCount = 0;

    std::ifstream fileStream;
    fileStream.open( gpPathToExecutable, std::ios_base::binary | std::ios_base::in );
    if ( !fileStream.is_open() || !fileStream.good() ) {
        OTDU_LOG_ALWAYS( "Failed to open executable '%s' for read!\n", gpPathToExecutable );
        return 1;
    }

    constexpr uint32_t kNumShaderEntries = sizeof( kMasterShaderTable ) / sizeof( kMasterShaderTable[0] );
  
    ShaderTable tableDXSO = {};
    ShaderTable tableGLSL = {};
    ShaderTable tableSPIRV = {};

    std::string currentTableName = "";
    int32_t dword = 0;
    for ( uint32_t i = 0; i < kNumShaderEntries; i++ ) {
        const ShaderTableEntry& entry = kMasterShaderTable[i];

        if (currentTableName != entry.pShaderCategory) {
            if (!tableDXSO.Header.Empty()) {
                std::string filename = currentTableName + kShaderTableExtension;
                
                std::string fileOutputPath = GetShaderOutputPath() + kShadersD3D9Folder + filename;
                bool bWroteToDisk = WriteShaderTableToDisk( tableDXSO, fileOutputPath );
                if (bWroteToDisk) {
                    shaderExtractedCount++;
                }

                fileOutputPath = GetShaderOutputPath() + kShadersVulkanFolder + filename;
                bWroteToDisk = WriteShaderTableToDisk( tableSPIRV, fileOutputPath );
                if ( bWroteToDisk ) {
                    shaderExtractedCount++;
                }

                if (!gbSkipGLSLTranslation) {
                    fileOutputPath = GetShaderOutputPath() + kShadersOpenGLFolder + filename;
                    bWroteToDisk = WriteShaderTableToDisk( tableGLSL, fileOutputPath );
                    if (bWroteToDisk) {
                        shaderExtractedCount++;
                    }
                } else {
                    OTDU_LOG_INFO( "Skipping GLSL translation ('-skip_glsl' found in command line)\n");
                }

                tableDXSO.Clear();
                tableSPIRV.Clear();
                tableGLSL.Clear();
            }
            currentTableName = entry.pShaderCategory;
        }
        size_t offsetToSeek = entry.OffsetInExecutable - 0x400000;
        fileStream.seekg( offsetToSeek, std::ios::beg );

        std::vector<int8_t> shaderBytecode;
        shaderBytecode.reserve( 2 << 10 ); // Should be large enough (biggest shaders are ~2KiB) 
        shaderBytecode.resize( shaderBytecode.size() + 0x18 );

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

            tableDXSO.Header.Add( std::make_tuple( entry.Hashcode, tableDXSO.Shaders.size() ) );
            tableDXSO.Shaders.insert(tableDXSO.Shaders.end(), shaderBytecode.begin(), shaderBytecode.end());

            // DXSO to SPIRV (using DXVK)
            dxvk::DxsoReader reader( reinterpret_cast< const char* >( shaderBytecode.data() ) );
            dxvk::DxsoModule dxvkModule( reader );
            dxvk::DxsoAnalysisInfo info = dxvkModule.analyze();

            dxvk::D3D9ConstantLayout constantLayout;
            constantLayout.floatCount = 128; // Arbitrary value for now (way overkill)

            dxvk::DxsoModuleInfo dxsoModuleInfo;
            dxvk::OpenTDUOutput* pSpirvBC = dxvkModule.compile( dxsoModuleInfo, std::to_string( entry.Hashcode ), info, constantLayout );
            tableSPIRV.Header.Add( std::make_tuple( entry.Hashcode, tableSPIRV.Shaders.size() ) );

            for ( uint32_t i = 0; i < pSpirvBC->Code.dwords(); i++ ) {
                uint32_t dword = pSpirvBC->Code.data()[i];

                tableSPIRV.Shaders.push_back( ( uint8_t )( dword & 0xff ) );
                tableSPIRV.Shaders.push_back( ( uint8_t )( dword >> 8 ) & 0xff );
                tableSPIRV.Shaders.push_back( ( uint8_t )( dword >> 16 ) & 0xff );
                tableSPIRV.Shaders.push_back( ( uint8_t )( dword >> 24 ) & 0xff );
            }
           
            if (!gbSkipGLSLTranslation) {
                // SPIRV to GLSL (using SPIRV-Cross)
                tableGLSL.Header.Add( std::make_tuple( entry.Hashcode, tableGLSL.Shaders.size() ) );
                
                spirv_cross::CompilerGLSL glsl( pSpirvBC->Code.data(), pSpirvBC->Code.dwords() );

                // Set some options.
                spirv_cross::CompilerGLSL::Options options;
                options.version = 330;
                options.es = true;
                options.vulkan_semantics = true;
                glsl.set_common_options( options );

                std::string sourceCode = glsl.compile();
                tableGLSL.Shaders.insert( tableGLSL.Shaders.end(), sourceCode.begin(), sourceCode.end() );
            }
        } else {
            OTDU_FATAL_ERROR("Invalid magic found (shader table entry might be invalid)\n");
        }
    }
    if ( !tableDXSO.Header.Empty() ) {
        std::string filename = currentTableName + kShaderTableExtension;

        std::string fileOutputPath = GetShaderOutputPath() + kShadersD3D9Folder + filename;
        bool bWroteToDisk = WriteShaderTableToDisk( tableDXSO, fileOutputPath );
        if ( bWroteToDisk ) {
            shaderExtractedCount++;
        }

        if ( !gbSkipGLSLTranslation ) {
            fileOutputPath = GetShaderOutputPath() + kShadersOpenGLFolder + filename;
            bool bWroteToDisk = WriteShaderTableToDisk( tableGLSL, fileOutputPath );
            if ( bWroteToDisk ) {
                shaderExtractedCount++;
            }
        } else {
            OTDU_LOG_INFO( "Skipping GLSL translation ('-skip_glsl' found in command line)\n" );
        }

        tableDXSO.Clear();
        tableGLSL.Clear();
    }
    fileStream.seekg(0, std::ios::beg);
    dword = 0;

    while ( true ) {
        size_t shaderOffset = fileStream.tellg();
        if ( !fileStream.read( reinterpret_cast< char* >( &dword ), sizeof( int32_t ) ) ) {
            break;
        }

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
    }
    fileStream.close();

    OTDU_LOG_ALWAYS( "==== Summary ====\nFound and extracted %u shader table(s) and %u .ini files to disk\n",
                    shaderExtractedCount, iniExtractedCount );

    return 0;
}
