#pragma once

class RenderScene;
class Instance;
class DrawList;
struct TransformMatrixCommand;
struct Material;
struct ConstantMap;
struct MaterialParameter;
struct Primitive;
struct AmbientLight;
struct DirectionalLight;
struct PointLight;
struct SpotLight;

struct BoundTextureProperty {
    uint32_t ID = 0;
    void* pTextureProperty = nullptr;
};

struct ConstantEntry {
    using Callback_t = std::function<Eigen::Vector4f*(void)>;

    struct Stage {
        uint32_t ConstantOffset = 0u;
        Callback_t pCallback = nullptr;
    };

    Stage EntryPerStage[2];
};

struct SceneRenderer {
    RenderScene* pActiveScene = nullptr;
    Instance* pActiveInstance = nullptr;
    DrawList* pActiveDrawList = nullptr;
    TransformMatrixCommand* pActiveTransform = nullptr;
    uint64_t ActiveShaderHashcode = 0ull;
    Material* pActiveMaterial = nullptr;
    ConstantMap* pActiveConstantMap0 = nullptr;
    ConstantMap* pActiveConstantMap1 = nullptr;
    uint32_t ActiveVSShaderInfos[2] = { 0x0, 0x0 };
    uint32_t ActivePSShaderInfos[2] = { 0x0, 0x0 };
    MaterialParameter* pActiveMaterialParameter = nullptr;
    std::vector<BoundTextureProperty> BoundTextureProperties;

    std::array<ConstantEntry, 16> ConstantsVectors;
    std::array<ConstantEntry, 16> ConstantsMatrices;
    
    uint32_t NumVectors[2];
    uint32_t NumMatrices[2];

    void reset();
    void bindPrimitiveInstance(Primitive* param_1, TransformMatrixCommand* param_2);

    void bindInstance(TransformMatrixCommand* param_2, Instance* param_3);
    void bindConstantsToGPU();
    void uploadLightsToGPU();

    void uploadLightsToCbuffer();

    Eigen::Vector4f computeLightAttenuation(float param_1,float param_2);

    void uploadDirectionalLight(DirectionalLight* param_1, int32_t param_2, bool param_3);
    void uploadAmbientLight(AmbientLight* param_1, int32_t param_2, bool param_3);
    void uploadPointLight(PointLight* param_1, int32_t param_2, bool param_3);
    void uploadSpotLight(SpotLight* param_1, int32_t param_2, bool param_3);

    Eigen::Vector4f& FUN_00513000();
    void applyInstanceColor(Instance* param_1, Eigen::Vector4f& ambient, Eigen::Vector4f& diffuse, Eigen::Vector4f& specular);
    void bindMaterialParameters();
};

extern SceneRenderer gSceneRenderer; // DAT_00fe77c0
