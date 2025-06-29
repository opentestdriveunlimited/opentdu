#pragma once

enum class eShaderType {
    ST_Pixel = 0,
    ST_Vertex,

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
    /* Common */
    #include "shader_table_default.inl"
    #include "shader_table_uva.inl"
    #include "shader_table_billboards.inl"
    #include "shader_table_flash.inl"
    #include "shader_table_bink.inl"
    #include "shader_table_noise.inl"
    #include "shader_table_impostors.inl"
    /* Skinning */
    #include "shader_table_skinning.inl"
    /* Terrain/Heightmap */
    #include "shader_table_terrain.inl"
    /* Lighting */
    #include "shader_table_lighting.inl"
    /* Vehicle Paint */
    #include "shader_table_vehicle_paint.inl"
    #include "shader_table_vehicle_paint_normalmapped.inl"
    #include "shader_table_vehicle_paint_normalmapped2.inl"
    /* Vehicle Materials */
    #include "shader_table_vehicle_materials.inl"
    #include "shader_table_vehicle_materials_worldspace.inl"
    #include "shader_table_vehicle_materials_tangentspace.inl"
    /* Vehicle Interior */
    #include "shader_table_vehicle_interior.inl"
    #include "shader_table_vehicle_dirt.inl"
    #include "shader_table_vehicle_common.inl"
    /* Vehicle Traffic */
    #include "shader_table_vehicle_traffic.inl"
    /* World */
    #include "shader_table_world.inl"
    #include "shader_table_world_roads.inl"
    #include "shader_table_world_buildings.inl"
    #include "shader_table_world_foliage.inl"
    #include "shader_table_world_flags.inl"
    /* Details */
    #include "shader_table_details.inl"
    /* Avatar */
    #include "shader_table_avatar.inl"
    /* PostFX */
    #include "shader_table_postfx.inl"
    /* Weather */
    #include "shader_table_weather_water.inl"
    #include "shader_table_weather_sun.inl"
    #include "shader_table_weather_clouds.inl"
    /* Tire Skids */
    #include "shader_table_tire_skids.inl"
    /* Ocean */
    #include "shader_table_ocean.inl"
    /* Shadow */
    #include "shader_table_shadow.inl"
};
