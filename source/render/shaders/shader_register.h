#pragma once

#include <unordered_map>

#include "render/material.h"
#include "render/gs_render_helper.h"

struct Material;
struct GPUShader;
struct ShaderTableEntry;
class RenderDevice;

static constexpr uint64_t kInvalidMetadataOffset = 0xffffffffffffffffull;

struct ShaderTableHeaderEntry
{
    uint64_t Hashcode; // Hashcode (from the game; not the actual bytecode hash)
    uint64_t Offset; // Offset in file (absolute)
    uint64_t Size; // Size (in bytes)
    uint64_t MetadataOffset; // Metadata offset in file (absolute). kInvalidMetadataOffset if unavailable/unused
};

// Infos extracted from DxvkShaderCreateInfo (only relevant infos for D3D9 are stored).
// NOTE: This struct does not include the binding reflection (stored right after NumBindings).
// During parsing, you must make sure to read 'sizeof(DxvkBindingInfo) * NumBindings' before reading
// the next metadata entry.
struct ShaderMetadataSPIRV 
{
    uint32_t InputMask;
    uint32_t OutputMask;
    uint32_t FlatShadingInputs;
    uint32_t SamplerPushConstantsSize; // samplerDwordCount * sizeof(uint32_t) 
    uint32_t NumBindings; // (SPIRV) Num of binding metadata (DxvkBindingInfo) to read
};

struct ShaderTableHeader
{
    size_t NumEntries;
    std::vector<ShaderTableHeaderEntry> Entries;

    inline size_t GetHeaderSize() const
    {
        return NumEntries * sizeof(ShaderTableHeaderEntry) + sizeof( size_t );
    }
    
    inline void Clear()
    {
        NumEntries = 0ull;
        Entries.clear();
    }

    inline bool Empty()
    {
        return NumEntries == 0ull;
    }

    inline void Add(ShaderTableHeaderEntry entry)
    {
        Entries.push_back(entry);
        NumEntries++;
    }
};

struct ShaderTable
{
    ShaderTableHeader   Header;
    std::vector<int8_t> Shaders;
    std::vector<int8_t> Metadata;

    inline void Clear()
    {
        Header.Clear();
        Shaders.clear();
        Metadata.clear();
    }
};

static constexpr const char* kShadersRootFolder = "EURO/Shaders/";
static constexpr const char* kShadersD3D9Folder = "/d3d9/";
static constexpr const char* kShadersOpenGLFolder = "/gl330/";
static constexpr const char* kShadersVulkanFolder = "/vulkan/";
static constexpr const char* kShaderTableExtension = ".bin";
static constexpr const char* kShaderTableSourceExtension = ".glsl";

struct ShaderBindingFlags // TODO: Rename me
{
    uint8_t bIsVertexShader;
    uint8_t Unknown;
    uint8_t Unknown2;
    uint8_t Unknown3;
    uint8_t UniformsLights[4]; // TODO: This is a limit from DX9; this could be removed once we move to modern gfx APIs
    uint8_t UniformsShadows[4]; // TODO: This is a limit from DX9; this could be removed once we move to modern gfx APIs
    uint8_t __PADDING__[20];
};
OTDU_SIZE_MUST_MATCH(ShaderBindingFlags, 0x20);

struct ShaderPermutationFlags 
{
    MaterialParameterFlags  PermutationFlags;
    ShaderBindingFlags      BindingFlags;
    uint64_t                PermutationHashcode;

    ShaderPermutationFlags();

    void clear();
    void initialize( MaterialParameter* param_1, uint8_t param_2 );
};

class ShaderRegister {
public:
    ShaderRegister();
    ~ShaderRegister();

    void registerMasterTable();
    void retrieveShadersForMaterial( Material* param_1, const uint32_t index );
    void registerShader( const ShaderTableEntry* pTableEntry );
    
    void releaseCachedShaderInstances();

    const GPUShader* getShader( RenderDevice* pDevice, eShaderType type, uint64_t hashcode );
    void invalidateShader( eShaderType type, uint64_t hashcode );

private:
    struct CachedShader 
    {
        GPUShader*      pShader = nullptr; // Shader instance
        void*           pShaderBin = nullptr; // Shader bytecode (or source for GLSL)
        uint64_t        ShaderBinSize = 0ull;
        uint32_t        Flag0 = 0u;
        uint32_t        Flag1 = 0u;
        uint32_t        Flag2 = 0u;
        uint32_t        Flag3 = 0u;
    };

private:
    void fillParameterFlags( Material* param_1, ShaderPermutationFlags* param_2 );
    bool retrieveVSPSForFlags( ShaderPermutationFlags& param_1, GPUShader* pOutVertexShader, GPUShader* pOutPixelShader );

private:
    Material* pDefaultMaterial;

    std::unordered_map<std::string, void*> shaderTableBinary;
    std::unordered_map<uint64_t, std::tuple<void*, uint64_t>> shaderBinaries;
    // Pointer to shader metadata/reflection. Only valid for SPIRV (for now).
    std::unordered_map<uint64_t, ShaderMetadataSPIRV*> shaderMetadata;

    std::unordered_map<uint64_t, CachedShader> cachedVertexShaders;
    std::unordered_map<uint64_t, CachedShader> cachedPixelShaders;

    uint32_t vertexShaderInstanceCount;
    uint32_t pixelShaderInstanceCount;
};

extern ShaderRegister gShaderRegister;
