#pragma once

class Instance;
struct Primitive;
class DrawList;
class SetupGraph;
struct LOD;
class LightSetupNode;
struct Material;

enum eRenderBucket : uint16_t {
    RB_Car                  = 0x00,
    RB_CarAlphaTest         = 0x01,
    RB_Avatar               = 0x02,
    RB_AvatarAlphaTest      = 0x03,
    RB_Building             = 0x04,
    RB_BuildingAlphaTest    = 0x05,
    RB_Vegetation           = 0x06,
    RB_VegetationAlphaTest  = 0x07,
    RB_Opaque               = 0x08,
    RB_AlphaTest            = 0x09,
    RB_Road                 = 0x0a,
    RB_Heightmap            = 0x0b,
    RB_HeightmapAlphaTest   = 0x0c,
    RB_HeightmapAlpha       = 0x0d,
    RB_WaterSea             = 0x0e,
    RB_Water                = 0x0f,
    RB_WaterFullAlpha       = 0x10,
    RB_Cloud                = 0x11,
    RB_CloudFullAlpha       = 0x12,
    RB_RoadPatch            = 0x13,
    RB_Shadow               = 0x14,
    RB_HardInst             = 0x15,
    RB_FullAlphaFar         = 0x16,
    RB_Impostor             = 0x17,
    RB_ImpostorFar          = 0x18,
    RB_TallGrass            = 0x19,
    RB_Electrical           = 0x1a,
    RB_AvatarAlpha          = 0x1b,
    RB_BuildingAlpha        = 0x1c,
    RB_BuildingFullAlpha    = 0x1d,
    RB_AvatarFullAlpha0     = 0x1e,
    RB_AvatarFullAlpha1     = 0x1f,
    RB_AvatarFullAlpha2     = 0x20,
    RB_AvatarFullAlpha3     = 0x21,
    RB_CarAlpha             = 0x22,
    RB_VegetationAlpha      = 0x23,
    RB_VegetationFullAlpha  = 0x24,
    RB_ShadowPlane          = 0x25,
    RB_Alpha                = 0x26,
    RB_ShadowInt            = 0x27,
    RB_FullAlpha            = 0x28,
    RB_Sun                  = 0x29,
    RB_FrontEnd             = 0x2a,
    RB_Video                = 0x2b,
    RB_Profile              = 0x2c,

    RB_Invalid              = 0x2d
};

static constexpr int32_t kNumRenderBuckets = 0x2d; // DAT_00faf470

struct RenderBucket {
    uint16_t            TransparencyOrder;
    const char*         pName;
    uint32_t            Flags;
    float               Time;
};

// DAT_00f71ef8
static RenderBucket gRenderBuckets[45] = {
    RenderBucket{ 0x00, "CAR",                      0, 0.0f },
    RenderBucket{ 0x01, "CAR_ALPHATEST",            0, 0.0f },
    RenderBucket{ 0x02, "AVATAR",                   0, 0.0f },
    RenderBucket{ 0x03, "AVATAR_ALPHATEST",         0, 0.0f },
    RenderBucket{ 0x04, "BUILDING",                 0, 0.0f },
    RenderBucket{ 0x05, "BUILDING_ALPHATEST",       0, 0.0f },
    RenderBucket{ 0x06, "VEGETATION",               8, 0.0f },
    RenderBucket{ 0x07, "VEGETATION_ALPHATEST",     8, 0.0f },
    RenderBucket{ 0x08, "OPAQUE",                   0, 0.0f },
    RenderBucket{ 0x09, "ALPHATEST",                0, 0.0f },
    RenderBucket{ 0x0a, "ROAD",                     0, 0.0f },
    RenderBucket{ 0x0b, "HEIGHTMAP",                0, 0.0f },
    RenderBucket{ 0x0c, "HEIGHTMAP_ALPHATEST",      0, 0.0f },
    RenderBucket{ 0x0d, "HEIGHTMAP_ALPHA",          0, 0.0f },
    RenderBucket{ 0x0e, "WATER_SEA",                0, 0.0f },
    RenderBucket{ 0x0f, "WATER",                    0, 0.0f },
    RenderBucket{ 0x10, "WATER_FULLALPHA",          0, 0.0f },
    RenderBucket{ 0x11, "CLOUD",                    0, 0.0f },
    RenderBucket{ 0x12, "CLOUD_FULLALPHA",          0, 0.0f },
    RenderBucket{ 0x13, "ROAD_PATCH",               0, 0.0f },
    RenderBucket{ 0x14, "SHADOW",                   0, 0.0f },
    RenderBucket{ 0x15, "HARDINST",                 0, 0.0f },
    RenderBucket{ 0x16, "FULLALPHA_FAR",            0, 0.0f },
    RenderBucket{ 0x17, "IMPOSTOR",                 0, 0.0f },
    RenderBucket{ 0x18, "IMPOSTOR_FAR",             0, 0.0f },
    RenderBucket{ 0x19, "TALLGRASS",                0, 0.0f },
    RenderBucket{ 0x1a, "ELECTRICAL",               0, 0.0f },
    RenderBucket{ 0x1b, "AVATAR_ALPHA",             0, 0.0f },
    RenderBucket{ 0x1c, "BUILDING_ALPHA",           0, 0.0f },
    RenderBucket{ 0x1d, "BUILDING_FULLALPHA",       0, 0.0f },
    RenderBucket{ 0x1e, "AVATAR_FULLALPHA_0",       0, 0.0f },
    RenderBucket{ 0x1f, "AVATAR_FULLALPHA_1",       0, 0.0f },
    RenderBucket{ 0x20, "AVATAR_FULLALPHA_2",       0, 0.0f },
    RenderBucket{ 0x21, "AVATAR_FULLALPHA_3",       0, 0.0f },
    RenderBucket{ 0x22, "CAR_ALPHA",                0, 0.0f },
    RenderBucket{ 0x23, "VEGETATION_ALPHA",         8, 0.0f },
    RenderBucket{ 0x24, "VEGETATION_FULLALPHA",     8, 0.0f },
    RenderBucket{ 0x25, "SHADOW_PLANE",             0, 0.0f },
    RenderBucket{ 0x26, "ALPHA",                    0, 0.0f },
    RenderBucket{ 0x27, "SHADOW_INT",               0, 0.0f },
    RenderBucket{ 0x28, "FULLALPHA",                0, 0.0f },
    RenderBucket{ 0x29, "SUN",                      0, 0.0f },
    RenderBucket{ 0x2a, "FRONTEND",                 4, 0.0f },
    RenderBucket{ 0x2b, "VIDEO",                    4, 0.0f },
    RenderBucket{ 0x2c, "PROFILE",                  4, 0.0f },
};

struct TransformMatrixCommand
{
    Eigen::Matrix4f                     ModelMatrix = Eigen::Matrix4f::Identity();
    int32_t                             IsInstanciated = 0xffffffff;
    uint32_t                            Unknown = 0;
    DrawList*                           pDrawList = nullptr;
    Instance*                           pInstance = nullptr;
    const SetupGraph*                   pSetup = nullptr;

    void execute(uint32_t isHeightmap, Primitive* param_3, LOD* param_4);
};

struct DrawCommand
{
    uint32_t                            IsHeightmap = 0;
    uint32_t                            Unknown = 0;
    Primitive*                          pPrimitive = nullptr;
    LOD*                                pActiveLOD = nullptr;
    std::list<TransformMatrixCommand>   Matrices;

    void execute(bool param_2, uint32_t param_3);
};

struct DrawCommandMaterial
{
    uint32_t                    Unknown = 0;
    float                       ZSort = 0.0f;
    Material*                   pMaterial = nullptr;
    std::list<DrawCommand>      DrawCommands;

    void execute();
};

struct SceneSetupCommand
{
    LightSetupNode*                     pLightSetupNode;
    std::list<DrawCommandMaterial>      MaterialCommands;

    void execute();
};

struct RenderBucketData
{
    uint32_t Index;
    uint32_t Flags;
    uint32_t Unknown;
    uint32_t Unknown2;

    std::list<DrawCommand>        DrawCommands;
    std::list<SceneSetupCommand>  SceneSetupCommands;

    SceneSetupCommand* allocateSceneSetupCommand(LightSetupNode* param_2);
};

void AllocateRenderBuckets();
void FreeRenderBuckets();
void AddPrimToBucket(Material* param_1, Instance* param_2, Primitive* param_3, const uint32_t isHeightmap, Eigen::Vector4f& boundingSphereAndRadius, LOD* param_6);
void AddPrimToBucket(Material* param_1, DrawList* param_2, const Eigen::Matrix4f& param_3, const Primitive* param_4, SetupGraph* param_5, const uint32_t isHeightmap);

extern RenderBucketData* gRenderBucketDatas;
