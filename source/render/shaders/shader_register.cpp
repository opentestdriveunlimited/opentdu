#include "shared.h"
#include "shader_register.h"

#include "shader_table_master.h"

#include "render/gs_render.h"
#include "render/material.h"
#include "core/hash/crc64.h"
#include "filesystem/gs_file.h"

#include "render/scene_renderer.h"

ShaderRegister gShaderRegister = {};

ShaderRegister::ShaderRegister()
    : pDefaultMaterial( nullptr )
    , vertexShaderInstanceCount( 0 )
    , pixelShaderInstanceCount( 0 )
{
    
}

ShaderRegister::~ShaderRegister()
{
    releaseCachedShaderInstances();

    for ( auto& table : shaderTableBinary ) {
        TestDrive::Free( table.second );
    }
    shaderTableBinary.clear();
}

void ShaderRegister::registerMasterTable()
{
    constexpr uint32_t kNumShaderEntries = sizeof( kMasterShaderTable ) / sizeof( kMasterShaderTable[0] );
    for ( uint32_t i = 0; i < kNumShaderEntries; i++ ) {
        registerShader( &kMasterShaderTable[i] );
    }
}

void ShaderRegister::retrieveShadersForMaterial( Material* param_1, const uint32_t index )
{
    OTDU_ASSERT( param_1 );

    if ( (void*)param_1->pVertexShaders[index] != nullptr ) {
        param_1->pVertexShaders[index] = nullptr;
    }

    if ( (void*)param_1->pPixelShaders[index] != nullptr ) {
        param_1->pPixelShaders[index] = nullptr;
    }

    ShaderPermutationFlags local_58;
    local_58.PermutationHashcode = 0x3dafe4f6b9bae7f4;
    if ( param_1->NumParams != 0 ) {
        fillParameterFlags( param_1, &local_58 );
        if ( local_58.PermutationHashcode == 0xcaca336d83078865 ) {
            local_58.PermutationFlags.NumPrelight = '\x01';
        }
    }

    gSceneRenderer.ActiveShaderHashcode = local_58.PermutationHashcode;
    
    bool bFoundPermutations = retrieveVSPSForFlags( local_58, param_1->pVertexShaders[index], param_1->pPixelShaders[index] );
    if ( !bFoundPermutations ) {
        // TODO: This is redundant; we might want to keep this in devbuild only to avoid unecessary
        // overhead
        uint64_t permutationHashcode = TestDriveCRC64( &local_58, sizeof( ShaderPermutationFlags ) );
        OTDU_LOG_WARN( "Failed to load shader(s) for permutation hashcode 0x%p; default material will be used\n", permutationHashcode );

        // TODO: I am not exactly sure what's PTR_DAT_00f47514 yet (I assume it's some kind of fallback/default
        // material)
        local_58.PermutationFlags.pCustomFlags[0] = param_1 == pDefaultMaterial /*PTR_DAT_00f47514*/;
        local_58.PermutationHashcode = 0x3dafe4f6b9bae7f4;

        bool bFoundPermutations = retrieveVSPSForFlags( local_58, param_1->pVertexShaders[index], param_1->pPixelShaders[index] );
        OTDU_ASSERT_FATAL( bFoundPermutations );
    }

    gSceneRenderer.ActiveShaderHashcode = local_58.PermutationHashcode;
}

void ShaderRegister::registerShader( const ShaderTableEntry* pTableEntry )
{
    auto it = shaderTableBinary.find( pTableEntry->pShaderCategory );
    if ( it == shaderTableBinary.end() ) {
        std::string shaderTablePath = kShadersRootFolder;
#if OTDU_D3D9
        shaderTablePath += kShadersD3D9Folder;
#elif OTDU_VULKAN
        shaderTablePath += kShadersVulkanFolder;
#elif OTDU_OPENGL
        shaderTablePath += kShadersOpenGLFolder;
#endif
        shaderTablePath += pTableEntry->pShaderCategory;
        shaderTablePath += kShaderTableExtension;
        
        void* pShaderTable = nullptr;
        uint32_t contentSize = 0;
        bool bFoundShaderTable = gpFile->loadFile( shaderTablePath.c_str(), &pShaderTable, &contentSize );
        if ( !bFoundShaderTable ) {
            OTDU_LOG_ERROR( "Failed to register shader '%s' (hashcode 0x%p): shader table '%s' does not exist\n", shaderTablePath.c_str(), pTableEntry->Hashcode, pTableEntry->pShaderCategory );
            return;
        }

        shaderTableBinary.insert( std::make_pair( pTableEntry->pShaderCategory, pShaderTable ) );

        ShaderTableHeader header;
        header.NumEntries = *( size_t* )pShaderTable;

        int8_t* pShaderContent = ( int8_t* )pShaderTable;

        ShaderTableHeaderEntry* pHeaderIt = ( ShaderTableHeaderEntry* )( ( int8_t* )pShaderTable + sizeof( size_t ) );
        for ( size_t i = 0; i < header.NumEntries; i++ ) {
            int8_t* pShaderPointer = pShaderContent + pHeaderIt->Offset;

            shaderBinaries.insert( std::make_pair( pHeaderIt->Hashcode, std::make_tuple( pShaderPointer, pHeaderIt->Size ) ) );

            if (pHeaderIt->MetadataOffset != kInvalidMetadataOffset) {
                ShaderMetadataSPIRV* pShaderMetadataPointer = ( ShaderMetadataSPIRV* )( pShaderContent + pHeaderIt->MetadataOffset );
                shaderMetadata.insert( std::make_pair( pHeaderIt->Hashcode, pShaderMetadataPointer ) );
            }
            pHeaderIt++;
        }

        OTDU_LOG_INFO( "Successfully parsed '%s' (found %ull shaders in table)\n", shaderTablePath.c_str(), header.NumEntries );
    }

    // Cache shader (note that we do not create the shader yet; this will be done on its first use)
    auto shaderBinIt = shaderBinaries.find( pTableEntry->Hashcode );
    if ( shaderBinIt == shaderBinaries.end() ) {
        OTDU_LOG_ERROR( "Failed to register shader 0x%p from shader table '%s': shader not found in hashtable!\n", pTableEntry->Hashcode, pTableEntry->pShaderCategory );
        return;
    }

    CachedShader cachedShader = { nullptr, std::get<0>( shaderBinIt->second ), std::get<1>( shaderBinIt->second ), pTableEntry->Flag0, pTableEntry->Flag1, pTableEntry->Flag2, pTableEntry->Flag3 };
    if ( pTableEntry->ShaderStage == eShaderType::ST_Vertex ) {
        cachedVertexShaders.insert( std::make_pair( pTableEntry->Hashcode, cachedShader ) );
    } else if ( pTableEntry->ShaderStage == eShaderType::ST_Pixel ) {
        cachedPixelShaders.insert( std::make_pair( pTableEntry->Hashcode, cachedShader ) );
    } else {
        OTDU_FATAL_ERROR( "Unimplemented shader stage %u!\n", pTableEntry->ShaderStage );
    }
}

void ShaderRegister::releaseCachedShaderInstances()
{
    if (gpRender != nullptr)
    {
        RenderDevice* pDevice = gpRender->getRenderDevice();
        if (pDevice != nullptr)
        {
            for ( auto& entry : cachedVertexShaders ) {
                CachedShader& shader = entry.second;
                if ( shader.pShader != nullptr ) {
                    pDevice->destroyShader( shader.pShader );
                    shader.pShader = nullptr;
                }
            }
            
            for ( auto& entry : cachedPixelShaders ) {
                CachedShader& shader = entry.second;
                if ( shader.pShader != nullptr ) {
                    pDevice->destroyShader( shader.pShader );
                    shader.pShader = nullptr;
                }
            }
        }
    }

    vertexShaderInstanceCount = 0u;
    pixelShaderInstanceCount = 0u;
}

const GPUShader* ShaderRegister::getShader( RenderDevice* pDevice, eShaderType type, uint64_t hashcode )
{
    CachedShader* pCacheEntry = nullptr;
    switch ( type ) {
    case eShaderType::ST_Pixel:
    {
        auto it = cachedPixelShaders.find( hashcode );
        pCacheEntry = ( it != cachedPixelShaders.end() ) ? &it->second : nullptr;
        break;
    }
    case eShaderType::ST_Vertex:
    {
        auto it = cachedVertexShaders.find( hashcode );
        pCacheEntry = ( it != cachedVertexShaders.end() ) ? &it->second : nullptr;
        break;
    }
    default:
        OTDU_FATAL_ERROR( "Invalid shader type %u!\n", type );
        break;
    }

    if ( pCacheEntry == nullptr ) {
        OTDU_LOG_ERROR( "Failed to find shader with hashcode %p (type: %u)\n", hashcode, type );
        return nullptr;
    }

    if ( pCacheEntry->pShader == nullptr ) {
        OTDU_LOG_INFO("Creating shader %p (type: %u)\n", hashcode, type );
        pCacheEntry->pShader = pDevice->createShader( type, pCacheEntry->pShaderBin, pCacheEntry->ShaderBinSize );

        if ( pCacheEntry->pShader == nullptr ) {
            OTDU_LOG_ERROR( "Failed to find shader with hashcode %p (type: %u)\n", hashcode, type );
            return nullptr;
        }
    }

    return pCacheEntry->pShader;
}

void ShaderRegister::invalidateShader( eShaderType type, uint64_t hashcode )
{
    RenderDevice* pDevice = gpRender->getRenderDevice();
    if (type == eShaderType::ST_Vertex) {
        auto it = cachedVertexShaders.find(hashcode);
        if (it != cachedVertexShaders.end()) {
            pDevice->destroyShader( it->second.pShader );
            it->second.pShader = nullptr;
            vertexShaderInstanceCount--;
        }
    } else if (type == eShaderType::ST_Pixel) {
        auto it = cachedPixelShaders.find(hashcode);
        if (it != cachedPixelShaders.end()) {
            pDevice->destroyShader( it->second.pShader );
            it->second.pShader = nullptr;
            pixelShaderInstanceCount--;
        }
    } else {
        OTDU_FATAL_ERROR( "Unimplemented shader stage %u!\n", type );
    }
}

void ShaderRegister::fillParameterFlags( Material* param_1, ShaderPermutationFlags* param_2 )
{
    if ( param_1->NumParams == 0 ) {
        return;
    }

    // TODO: Cleanup all those crappy hardcoded offsets
    MaterialParameter* pParameters = ( MaterialParameter* )( (int8_t*)param_1 + 0xd0 + 0x10 );
    if ( pParameters->Type == 1 ) {
        MaterialTerrainParameter* pParameter = ( MaterialTerrainParameter* )pParameters;

        param_2->PermutationHashcode = 0x30c13b48948a3573;
        param_2->PermutationFlags.NumTextures = pParameter->Flags.NumTextures;
        param_2->PermutationFlags.pCustomFlags[0] = pParameter->Flags.pCustomFlags[0];
        param_2->PermutationFlags.pCustomFlags[1] = pParameter->Flags.pCustomFlags[1];
    } else if ( pParameters->Type == 2 ) {
        MaterialShaderParameterArray* pParameter = ( MaterialShaderParameterArray* )pParameters;

        param_2->clear();
        param_2->PermutationHashcode = pParameter->Hashcode;
        param_2->BindingFlags.bIsVertexShader = true;
            
        MaterialLayer* pLayer = pParameter->getLayer( 0 );

        uint8_t numTextures = pLayer->getNumUsedTextureSlots();
        param_2->initialize( pParameter, numTextures );

        switch ( param_2->PermutationHashcode ) {
        case 0x1010101010101010:
        {
            param_2->PermutationFlags.NumUVMaps = 4;
            param_2->PermutationFlags.Unknown = 0xf;
            break;
        }
        case 0x1c6c7357183157b8:
        {
            uint32_t numUVMaps = param_2->PermutationFlags.pCustomFlags[0]
                                + param_2->PermutationFlags.pCustomFlags[1];
            param_2->PermutationFlags.NumUVMaps = numUVMaps;

            if ( param_2->PermutationFlags.NumUVMaps == 0
                && param_2->PermutationFlags.bUsingGlossMap ) {
                param_2->PermutationFlags.NumUVMaps = 1;
            }
            break;
        }
        case 0x5789e65cb39c3501:
        {
            param_2->PermutationFlags.NumUVMaps = param_2->PermutationFlags.pCustomFlags[0];

            if ( param_2->PermutationFlags.pCustomFlags[1] != 0
                    || ( param_2->PermutationFlags.pCustomFlags[3] != 0
                        || param_2->PermutationFlags.bUsingGlossMap ) ) {
                param_2->PermutationFlags.NumUVMaps = param_2->PermutationFlags.pCustomFlags[0] + 1;
            }
            break;
        }
        case 0xa00d5c4ecab12190:
        {
            if ( !param_2->PermutationFlags.bUsingReflection ) {
                param_2->PermutationFlags.pCustomFlags[2] = '\0';
                param_2->PermutationFlags.pCustomFlags[4] = '\0';
                param_2->PermutationFlags.bUsingNormals = '\0';
            }
            break;
        }
        case 0xdb79dacbd5013ed0:
        {
            param_2->PermutationFlags.NumUVMaps = 0;

            if ( param_2->PermutationFlags.pCustomFlags[0] != '\0'
                    || param_2->PermutationFlags.pCustomFlags[1] != 0 ) {
                param_2->PermutationFlags.NumUVMaps = 1;
            }

            if ( param_2->PermutationFlags.bUsingGlossMap ) {
                param_2->PermutationFlags.NumUVMaps++;
            }
            break;
        }
        case 0xdb79baedd5013ed0:
        {
            param_2->PermutationFlags.bUsingSkinning = '\x01';
            param_2->PermutationFlags.bUsingNormals = '\x01';
            param_2->PermutationFlags.NumPrelight = '\0';
            param_2->PermutationFlags.NumUVMaps = '\x01';
            param_2->PermutationFlags.NumTextures = '\x01';
            break;
        }
        case 0x27c8abacd453201:
        {
            pParameter->Flags.pCustomFlags[0] = 1;
            pParameter->Flags.pCustomFlags[1] = 1;
            pParameter->Flags.pCustomFlags[2] = 1;
            pParameter->Hashcode = 0xca37336d73078865;

            param_2->PermutationFlags.pCustomFlags[0] = '\x01';
            param_2->PermutationFlags.pCustomFlags[1] = '\x01';
            param_2->PermutationFlags.pCustomFlags[2] = '\x01';
            param_2->PermutationHashcode = 0xca37336d73078865;

            param_1->FXFlags = param_1->FXFlags & 0xfffffffb;
            break;
        }
        };
    }
}

bool ShaderRegister::retrieveVSPSForFlags( ShaderPermutationFlags& param_1, GPUShader* pOutVertexShader, GPUShader* pOutPixelShader )
{
    bool bVar2 = true;
    if ( pOutVertexShader != nullptr ) {
        param_1.BindingFlags.bIsVertexShader = true;
        uint64_t permutationHashcode = TestDriveCRC64( &param_1, sizeof( ShaderPermutationFlags ) );

        auto it = cachedVertexShaders.find( permutationHashcode );
        if ( it != cachedVertexShaders.end() ) {
            pOutVertexShader = it->second.pShader;
        } else {
            bVar2 = false;
        }
    }

    if ( pOutPixelShader != nullptr ) {
        param_1.BindingFlags.bIsVertexShader = false;
        uint64_t permutationHashcode = TestDriveCRC64( &param_1, sizeof( ShaderPermutationFlags ) );

        auto it = cachedPixelShaders.find( permutationHashcode );
        if ( it != cachedPixelShaders.end() ) {
            pOutPixelShader = it->second.pShader;
        } else {
            bVar2 = false;
        }
    }

    return bVar2;
}

ShaderPermutationFlags::ShaderPermutationFlags()
{
    clear();
}

void ShaderPermutationFlags::clear()
{
    memset( this, 0, 0x58 );

    // TODO: The ctor for this struct (0x00603000) sets the VS flag to true on init. Why??
    BindingFlags.bIsVertexShader = true;
}

void ShaderPermutationFlags::initialize( MaterialParameter* param_1, uint8_t param_2 )
{
    PermutationFlags.NumTextures = param_2;
    PermutationFlags.NumUVMaps = param_1->Flags.NumUVMaps;
    PermutationFlags.bUsingSkinning = param_1->Flags.bUsingSkinning;
    PermutationFlags.bUseTextureMatrix = param_1->Flags.bUseTextureMatrix;
    PermutationFlags.NumPrelight = param_1->Flags.NumPrelight;
    PermutationFlags.bUsingNormals = param_1->Flags.bUsingNormals;
    PermutationFlags.bAffectedByFog = param_1->Flags.bAffectedByFog;
    PermutationFlags.bUnknown2 = param_1->Flags.bUnknown2;
    PermutationFlags.NumLightmaps = param_1->Flags.NumLightmaps;
    PermutationFlags.NumFXMap = param_1->Flags.NumFXMap;
    PermutationFlags.bUsingAmbient = param_1->Flags.bUsingAmbient;
    PermutationFlags.bUsingDiffuse = param_1->Flags.bUsingDiffuse;
    PermutationFlags.bUsingSpecular = param_1->Flags.bUsingSpecular;
    PermutationFlags.bUsingGlossMap = param_1->Flags.bUsingGlossMap;
    PermutationFlags.bUsingOffsetMap = param_1->Flags.bUsingOffsetMap;
    PermutationFlags.bUsingReflection = param_1->Flags.bUsingReflection;
    PermutationFlags.bUsingEmissive = param_1->Flags.bUsingEmissive;
    PermutationFlags.bUsingBinormal = param_1->Flags.bUsingBinormal;
    PermutationFlags.bUsingSpriteRotation = param_1->Flags.bUsingSpriteRotation;
    PermutationFlags.bUsingDepthSprite = param_1->Flags.bUsingDepthSprite;
    PermutationFlags.Unknown = param_1->Flags.Unknown;
    PermutationFlags.bUsingTangentSpace = param_1->Flags.bUsingTangentSpace;
    PermutationFlags.bUsingBumpMap = param_1->Flags.bUsingBumpMap;
    PermutationFlags.bUsingTangent = param_1->Flags.bUsingTangent;
    PermutationFlags.pCustomFlags[0] = param_1->Flags.pCustomFlags[0];
    PermutationFlags.pCustomFlags[1] = param_1->Flags.pCustomFlags[1];
    PermutationFlags.pCustomFlags[2] = param_1->Flags.pCustomFlags[2];
    PermutationFlags.pCustomFlags[3] = param_1->Flags.pCustomFlags[3];
    PermutationFlags.pCustomFlags[4] = param_1->Flags.pCustomFlags[4];
    PermutationFlags.pCustomFlags[5] = param_1->Flags.pCustomFlags[5];
    PermutationFlags.pCustomFlags[6] = param_1->Flags.pCustomFlags[6];
    PermutationFlags.pCustomFlags[7] = param_1->Flags.pCustomFlags[7];
}
