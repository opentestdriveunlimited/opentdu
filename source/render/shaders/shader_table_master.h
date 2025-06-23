#pragma once

enum class eShaderType {
    ST_Vertex = 0,
    ST_Pixel,

    ST_Count,

    ST_Invalid = -1
};

struct ShaderTableEntry
{
    uint64_t        OffsetInExecutable = 0ull;
    uint64_t        Hashcode = 0ull;
    eShaderType     ShaderStage = eShaderType::ST_Invalid;
    uint32_t        Flag0 = 0u;
    uint32_t        Flag1 = 0u;
    uint32_t        Flag2 = 0u;
    uint32_t        Flag3 = 0u;
    const char*     pShaderCategory = "Unknown";
};

static constexpr ShaderTableEntry kMasterShaderTable[] = {
    #include "shader_table_default.inl"
    #include "shader_table_lighting.inl"
    #include "shader_table_flash.inl"
    #include "shader_table_terrain.inl"
};
