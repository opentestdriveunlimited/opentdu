#pragma once

#include <unordered_map>

#include "render/material.h"

struct Material;
struct GPUShader;
struct ShaderTableEntry;

using ShaderTableHeaderEntry_t = std::tuple<uint64_t, uint64_t>; // Key: Hashcode Value: Offset in file (absolute)
struct ShaderTableHeader
{
    size_t NumEntries;
    std::vector<ShaderTableHeaderEntry_t> Entries;

    inline size_t GetHeaderSize() const
    {
        return NumEntries * (sizeof(uint64_t) * 2) + sizeof( size_t );
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

    inline void Add(ShaderTableHeaderEntry_t entry)
    {
        Entries.push_back(entry);
        NumEntries++;
    }
};

struct ShaderTable
{
    ShaderTableHeader   Header;
    std::vector<int8_t> Shaders;

    inline void Clear()
    {
        Header.Clear();
        Shaders.clear();
    }
};

static constexpr const char* kShadersRoot = "EURO/Shaders/";

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
static_assert( sizeof( ShaderBindingFlags ) == 0x20, "Size mismatch! Shader permutation deserialization will fail" );

struct ShaderPermutationFlags 
{
    MaterialParameterFlags  PermutationFlags;
    ShaderBindingFlags      BindingFlags;
    uint64_t                PermutationHashcode;

    ShaderPermutationFlags();

    void clear();
    void initialize( MaterialParameter* param_1, uint8_t param_2 );
};

struct CachedShader
{
    GPUShader*      pShader = nullptr;
    uint32_t        Flag0 = 0u;
    uint32_t        Flag1 = 0u;
    uint32_t        Flag2 = 0u;
    uint32_t        Flag3 = 0u;
};

class ShaderRegister {
public:
    ShaderRegister();

    void registerMasterTable();
    void retrieveShadersForMaterial( Material* param_1, const uint32_t index );
    void registerShader( const ShaderTableEntry* pTableEntry );

private:
    void fillParameterFlags( Material* param_1, ShaderPermutationFlags* param_2 );
    bool retrieveVSPSForFlags( ShaderPermutationFlags& param_1, CachedShader* pOutVertexShader, CachedShader* pOutPixelShader );

private:
    uint64_t latestFoundHashcode;

    Material* pDefaultMaterial;

    std::unordered_map<uint64_t, CachedShader> cachedVertexShaders;
    std::unordered_map<uint64_t, CachedShader> cachedPixelShaders;
};

extern ShaderRegister gShaderRegister;
