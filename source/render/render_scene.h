#pragma once

class Camera;
struct Viewport;
class RenderTarget;
class DrawList;
class Instance;
struct HiearchyNode;
struct InstanceWithCustomMaterial;
struct FramebufferAttachments;
struct RenderObject;
struct Heightmap;
class LightSetupNode;
struct Material;

#include "frustum.h"
#include "render_object_base.h"
#include "setup_node.h"

enum class eFogType : uint32_t {
    FT_None = 0,
    FT_Exponential = 1,
    FT_ExponentialSquared = 2,
    FT_Linear = 3,
    FT_Invalid = 0xffffffff
};

struct FogDesc {
    eFogType Type = eFogType::FT_None;
    Eigen::Vector4f Color = { 0.0f, 0.0f, 0.0f, 1.0f };
    float DistanceStart = 0.0f;
    float DistanceEnd = 0.0f;
    float Intensity = 1.0f;
};

struct RenderSceneCommands {
    std::vector<DrawList*>      StaticDrawLists;
    std::vector<Instance*>      StaticInstances;
    std::vector<HiearchyNode*>  StaticHiearchies;

    std::vector<DrawList*>      DynamicDrawLists;
    std::vector<Instance*>      DynamicInstances;
    std::vector<HiearchyNode*>  DynamicHiearchies;

    std::vector<InstanceWithCustomMaterial*> DynamicInstancesWithCustomMat;
    std::vector<std::vector<Instance*>*>     DynamicInstancesArrays;

    uint32_t RefCount = 0u;
};

class RenderScene : public RenderObjectBase {
public:
    inline void setUnknownMask(const uint64_t value) { unknownMask = value; }
    inline uint64_t getUnknownMask() const { return unknownMask; }
    inline FogDesc& getFogDescWrite() { return fogDesc; }
    inline Frustum& getFrustumWrite() { return frustum; }
    inline uint32_t getFlags() const { return flags; }
    inline void setFlags(uint32_t newFlags) { flags = newFlags; }
    inline Camera* getCamera() { return pActiveCamera; }
    inline FramebufferAttachments* getFramebuffer() { return framebufferAttachments; }
    inline Viewport* getViewport() { return pActiveViewport; }
    inline bool isOrthoProjection() const { return bOrthoProjection; }
    inline SetupGraph& getSetup() { return sceneSetup; }

public:
    RenderScene();
    ~RenderScene();

    void destroy();

    void create(Camera *param_2, Viewport *param_3,FramebufferAttachments *param_4,uint32_t param_5,const char *param_6,bool param_7,RenderScene * param_8,RenderScene *param_9);
    void copy(RenderScene *param_2,const char *param_3);
    void enqueueDynamicDrawList(DrawList* pDrawList);

    void setSceneType(bool param_1);
    void setShaderIndexes(uint32_t vsIndex, uint32_t psIndex);

    void addLightSetup(LightSetupNode* param_2);
    void copyDrawCommands(RenderScene *param_2);

    void removeDynamicDrawCommands();

    void submitDrawCommands( uint32_t param_2 );

private:
    Frustum     frustum;
    FogDesc     fogDesc;
    Camera*     pActiveCamera;
    Viewport*   pActiveViewport;
    uint32_t    flags;
    uint64_t    unknownMask;

    uint32_t    activeVertexShaderIndex;
    uint32_t    activePixelShaderIndex;
    
    RenderSceneCommands* pDrawCommands;

    SetupGraph sceneSetup;

    // TODO: Not sure what is it used
    std::vector<Eigen::Vector4f*> vectorRegister;

    FramebufferAttachments* framebufferAttachments;

    uint8_t     bOrthoProjection : 1;

private:
    void initialize(Camera *param_2, Viewport *param_3,FramebufferAttachments *param_4,uint32_t param_5,const char *param_6);
    void freeDrawCommands();

    void submitDrawListToBucket(DrawList* param_2);
    void submitInstance(Instance* param_2);
    void submitHeightmap(Heightmap* param_1, Instance* param_2);
    void submitObject(RenderObject* param_1, Instance* param_2);
    void submitInstanceWithCustomMaterial(RenderObject* param_2, InstanceWithCustomMaterial* param_3);

    bool isInstanceVisible(RenderObject* param_1, Instance* param_2);
    bool FUN_00508930(Material* param_2) const;
};
