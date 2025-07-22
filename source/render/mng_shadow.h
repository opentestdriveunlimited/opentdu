#pragma once

#include "game/mng_base.h"
#include "render/file_collection_2d.h"

struct RenderTarget;

class MngShadow : public Manager {
public:
    const char* getName() const override { return "Shadow"; }

public:
    MngShadow();
    ~MngShadow();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float ) override;
    virtual void draw() override;
    virtual void terminate()  override;

private:
    struct ShadowMapResources {
        Eigen::Vector2i Dimensions = { 512, 512 };
        RenderTarget* pRenderTarget = nullptr;
        void* pMemory = nullptr;
        Render2DB Bitmap = {};
    };

private:
    FileCollection2D collection2D;
    ShadowMapResources exteriorShadowMap;
    ShadowMapResources interiorShadowMap;
};
    
extern MngShadow* gpMngShadow;
