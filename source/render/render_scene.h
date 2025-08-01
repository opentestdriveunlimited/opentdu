#pragma once

class Camera;
struct Viewport;
class RenderTarget;
class DrawList;
struct Instance;
struct HiearchyNode;
struct InstanceWithCustomMaterial;

#include "frustum.h"

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

class RenderScene {
public:
    inline void setUnknownMask(const uint64_t value) { unknownMask = value; }
    inline FogDesc& getFogDescWrite() { return fogDesc; }

public:
    RenderScene();
    ~RenderScene();

    void enqueueDynamicDrawList(DrawList* pDrawList);

private:
    Frustum     frustum;
    FogDesc     fogDesc;
    Camera*     pActiveCamera;
    Viewport*   pActiveViewport;
    uint32_t    flags;
    uint64_t    unknownMask;

    uint32_t    activeVertexShaderIndex;
    uint32_t    activePixelShaderIndex;
    
    RenderSceneCommands drawCommands;

    // TODO: Not sure what is it used
    std::vector<Eigen::Vector4f*> vectorRegister;

    std::vector<RenderTarget*> framebufferAttachments;

    uint8_t     bOrthoProjection : 1;
};
