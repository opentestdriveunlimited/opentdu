#pragma once

#include "game/mng_base.h"
#include "core/color.h"

class Camera;
class RenderScene;
struct HiearchyNode;
class Instance;

class MngReflection : public Manager {
public:
    static ColorRGBA ReflectionBackgroundColor;

public:
    const char* getName() const override { return "Shadow"; }

public:
    MngReflection();
    ~MngReflection();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float ) override {}
    virtual void draw() override;
    virtual void terminate()  override;
    virtual void reset() override;
    virtual void pause() override;

    void removeNode( HiearchyNode* pNode );

private:
    RenderScene* pScene;
    Camera* pCamera;

    std::vector<HiearchyNode*> hiearchyNodes;
    std::vector<Instance*> instances;

    Eigen::Vector3f planeNormal;
    float planeOffset;

    Eigen::Vector4f clipPlanes[6];
    
    uint32_t clearFlagsBackup;
    int32_t activeClipPlane;
    uint8_t bEnableClip : 1;
    uint8_t bInitialized : 1;
};

extern MngReflection* gpMngReflection;
