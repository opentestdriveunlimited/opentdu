#pragma once

#include "game/mng_base.h"
#include "core/color.h"
#include "render/2dm.h"
#include "render/file_collection_2d.h"

class DrawList;
struct Material;

class MngFade : public Manager {
public:
    using PostTransitionCallback_t = std::function<void(void*, void*)>;

public:
    const char* getName() const override { return "MngFade"; }

public:
    MngFade();
    virtual ~MngFade();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;
    virtual void draw() override;
    virtual void pause() override;

private:
    FileCollection2D fileCollection;

    DrawList* pDrawListTransition;
    ColorRGBA transitionColor;
    uint32_t flags;
    float transitionDuration;

    PostTransitionCallback_t pCallback;
    void* pCallbackSource;
    void* pCallbackData;
    
    Eigen::Vector2f position;
    float width;
    float height;
    float elapsedTime;
    float logoElapsedTime;

    DrawList* pDrawListLogo;
    Render2DM transition2DM;
    void* p2DMBuffer;
    
    Material* pMaterial;

    uint8_t bRenderLogo : 1;
    uint8_t bWaitForPopup : 1;

private:
    void encodeTransitionDrawList(const uint8_t alpha, const float fVar7);
    void encodeLogoDrawList();
};
