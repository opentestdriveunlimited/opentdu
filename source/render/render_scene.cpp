#include "shared.h"
#include "render_scene.h"

#include "render/light.h"
#include "render/gs_render.h"
#include "render/frame_graph.h"

RenderScene::RenderScene()
    : RenderObjectBase()
    , frustum()
    , fogDesc()
    , pActiveCamera( nullptr )
    , pActiveViewport( nullptr )
    , flags( 0u )
    , unknownMask( 0xffffffffffffffff )
    , activeVertexShaderIndex( 0u )
    , activePixelShaderIndex( 0u )
    , pDrawCommands( nullptr)
    , bOrthoProjection( false )
{

}

RenderScene::~RenderScene()
{
    
}

void RenderScene::destroy()
{
    // FUN_0050b1e0
    sceneSetup.removeLightNodes();
    sceneSetup.removeFrustumNodes();

    freeDrawCommands();
    gpRender->getFrameGraph()->removeObject(this);
}

void RenderScene::create(Camera * param_2, Viewport * param_3, FramebufferAttachments * param_4, uint32_t param_5, const char * param_6, bool param_7, RenderScene * param_8, RenderScene * param_9)
{
    // FUN_0050b140
    if (param_9 == nullptr) {
        initialize(param_2,param_3,param_4,param_5,param_6);
    } else {
        copy(param_9,param_6);
        flags = flags | param_5;
        if (param_2 != nullptr) {
            pActiveCamera = param_2;
            frustum.setCamera(param_2, bOrthoProjection);
        }
        if (param_3 != nullptr) {
            pActiveViewport = param_3;
        }
        if (param_4 != nullptr) {
            framebufferAttachments = param_4;
        }
    }

    if (param_8 != nullptr) {
        gpRender->getFrameGraph()->insertAfterObject(this, param_8);
        return;
    } 
    
    gpRender->getFrameGraph()->pushObject(this, param_7);
}

void RenderScene::enqueueDynamicDrawList(DrawList *pDrawList)
{
    pDrawCommands->DynamicDrawLists.push_back(pDrawList);
}

void RenderScene::initialize(Camera * param_2, Viewport * param_3, FramebufferAttachments * param_4, uint32_t param_5, const char * param_6)
{
    // FUN_00508ac0
    setName((param_6 != nullptr) ? param_6 : "Scn-Anonymous");

    pActiveCamera = param_2;
    frustum.setCamera(param_2, false);
    pActiveViewport = param_3;
    framebufferAttachments = param_4;
    bOrthoProjection = false;
    flags = param_5;
    
    pDrawCommands = new RenderSceneCommands();
    pDrawCommands->RefCount++;
}

void RenderScene::copy(RenderScene * param_2, const char * param_3)
{
    // FUN_00508b90
    setName((param_3 != nullptr) ? param_3 : param_2->getName());

    pActiveCamera = param_2->pActiveCamera;
    pActiveViewport = param_2->pActiveViewport;
    framebufferAttachments = param_2->framebufferAttachments;
    flags = 0;
    frustum.setCamera(param_2->pActiveCamera, param_2->bOrthoProjection);
    bOrthoProjection = param_2->bOrthoProjection;
    flags = flags | param_2->flags;

    if (pDrawCommands != nullptr) {
        pDrawCommands->RefCount--;
    }

    pDrawCommands = param_2->pDrawCommands;
    pDrawCommands->RefCount++;
}

void RenderScene::setSceneType(bool param_1)
{
    bOrthoProjection = param_1;
    frustum.setCamera(pActiveCamera, bOrthoProjection);
}

void RenderScene::setShaderIndexes(uint32_t vsIndex, uint32_t psIndex)
{
    activeVertexShaderIndex = vsIndex;
    activePixelShaderIndex = psIndex;
}

void RenderScene::addLightSetup(LightSetupNode * param_2)
{
    // FUN_0050b110
    if (param_2 != nullptr) {
        sceneSetup.addNode(param_2);
    }
}

void RenderScene::copyDrawCommands(RenderScene * param_2)
{
    // FUN_00508a90
    if (param_2 != nullptr) {
        if (pDrawCommands != nullptr) {
            pDrawCommands->RefCount--;
        }
        pDrawCommands = param_2->pDrawCommands;
        pDrawCommands->RefCount++;
    }
}

void RenderScene::freeDrawCommands()
{
    // FUN_00508a40
    if (pDrawCommands != nullptr && pDrawCommands->RefCount != 0) {
        pDrawCommands->RefCount--;
    }

    pDrawCommands = nullptr;
}