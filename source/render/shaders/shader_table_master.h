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
    {0x009f2f80,0x6c6cd946d32bc9da,eShaderType::ST_Vertex,0,0,2,0,"DownscaleVS"},
    #include "shader_table_default.inl"
    #include "shader_table_lighting.inl"
    #include "shader_table_flash.inl"
    #include "shader_table_terrain.inl"
    #include "shader_table_vehicle_paint.inl"
    #include "shader_table_vehicle_paint_normalmapped.inl"
    #include "shader_table_vehicle_paint_normalmapped2.inl"
    #include "shader_table_vehicle_materials.inl"
    #include "shader_table_vehicle_materials_worldspace.inl"
    #include "shader_table_vehicle_materials_tangentspace.inl"
    #include "shader_table_uva.inl"
    #include "shader_table_billboards.inl"
    #include "shader_table_world_roads.inl"
    #include "shader_table_skinning.inl"
    #include "shader_table_world_buildings.inl"
    #include "shader_table_world.inl"
    #include "shader_table_world_foliage.inl"
    #include "shader_table_impostors.inl"
    #include "shader_table_weather_water.inl"
    #include "shader_table_shadow.inl"
    #include "shader_table_weather_sun.inl"
    {0x00ddaab0,0xab9ec561e383b572,eShaderType::ST_Vertex,0,0,0x80000000,0,"Particles"},
    {0x00ddac08,0x2b069292ccae3f6f,eShaderType::ST_Pixel,0x80000132,0,0,0,"Particles"}, 
    {0x00e2fb58,0x3129566dded49e1a,eShaderType::ST_Pixel,1,0,0,0,"DepthPrepass"},
    {0x00e2fc48,0x444317b1d27aaf0c,eShaderType::ST_Pixel,1,0,0,0,"DepthPrepass"},
    #include "shader_table_avatar.inl"
    #include "shader_table_weather_clouds.inl"
    #include "shader_table_vehicle_interior.inl"
    #include "shader_table_tire_skids.inl"
    #include "shader_table_world_flags.inl"
    #include "shader_table_details.inl"
};
