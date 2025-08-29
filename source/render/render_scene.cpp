#include "shared.h"
#include "render_scene.h"

#include "render/light.h"
#include "render/gs_render.h"
#include "render/frame_graph.h"
#include "render/instance.h"
#include "render/3dd.h"
#include "render/draw_list.h"
#include "render/2dm.h"
#include "render/material.h"

#include "core/mutex.h"

static TestDriveMutex gDynamicDLMutex;

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
    ScopedMutexLock lock(&gDynamicDLMutex);
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

void RenderScene::removeDynamicDrawCommands()
{
    // FUN_00508f10
    pDrawCommands->DynamicDrawLists.clear();
    pDrawCommands->DynamicHiearchies.clear();
    pDrawCommands->DynamicInstances.clear();
    pDrawCommands->DynamicInstancesArrays.clear();
    pDrawCommands->DynamicInstancesWithCustomMat.clear();
}

void RenderScene::submitDrawCommands(uint32_t param_2)
{
    // FUN_00508c10
    gpRender->beginRenderScene(this);

    for (DrawList* peVar2 : pDrawCommands->StaticDrawLists) {
        submitDrawListToBucket(peVar2);
    }

    for (Instance* peVar2 : pDrawCommands->StaticInstances) {
        OTDU_UNIMPLEMENTED;
    }

    for (HiearchyNode* peVar2 : pDrawCommands->StaticHiearchies) {
        OTDU_UNIMPLEMENTED;
    }

    for (DrawList* peVar2 : pDrawCommands->DynamicDrawLists) {
        submitDrawListToBucket(peVar2);
    }
    
    for (Instance* peVar2 : pDrawCommands->DynamicInstances) {
        OTDU_UNIMPLEMENTED;
    }
    
    for (InstanceWithCustomMaterial* peVar6 : pDrawCommands->DynamicInstancesWithCustomMat) {
        Instance* peVar7 = peVar6->pInstance;
        if (((peVar7->getFlags() >> 2 & 1) == 0) && (peVar7->getNumLODs() != 0)) {
            if ((peVar7->getFlags() >> 1 & 1) != 0) {
                peVar7->calculateLOD(gpRender->getActiveCamera());
            }

            if (peVar7->getActiveLODIndex() < kMaxNumLOD) {
                void* piVar8 = peVar7->getLOD(peVar7->getActiveLODIndex()).pUnknown;

                uint32_t pMagic = *(uint32_t*)piVar8;
                if (pMagic == kObjectMagic) {
                    RenderObject* pObj = (RenderObject*)((uint8_t*)piVar8 + 4);
                    OTDU_UNIMPLEMENTED;
                }
            }
        }
    }
    
    for (auto& peVar10 : pDrawCommands->DynamicInstancesArrays) {
        for (Instance* peVar2 : *peVar10) {
            OTDU_UNIMPLEMENTED;
        }
    }
    
    for (HiearchyNode* peVar2 : pDrawCommands->DynamicHiearchies) {
        OTDU_UNIMPLEMENTED;
    }
}

void RenderScene::submitDrawListToBucket(DrawList * param_2)
{
    // FUN_00508960
    for (uint32_t iVar6 = 0; iVar6 < param_2->getNumPrimitives(); iVar6++) {
        Material* peVar7 = param_2->getMaterialAtIndex( iVar6 );
        if (peVar7 == nullptr) {
            peVar7 = DAT_00f4751c; // TODO: I guess this is the fallback/pink material?
        }

        uint32_t* peVar4 = param_2->getFlagsAtIndex( iVar6 );
        if ((peVar7->FXFlags >> 3) & 1 != 0 
        && ((1ull << peVar7->OT) & gpRender->getActiveScene()->getUnknownMask())) { // TODO: Can we simplify and use 'this' instead?
            OTDU_UNIMPLEMENTED;
            // AddPrimToBucket(peVar7,pRVar5,(float *)((int)pTVar3->Scalars[0] + local_14),
            //                 &(param_2->Streams).Normals + (int)&(peVar2->Chunk).Type,peVar4,0);
        }
    }
}

void RenderScene::submitInstance(Instance * param_2)
{
    // FUN_00509390
    const uint32_t flags = param_2->getFlags();
    if (((flags >> 2 & 1) == 0) && (param_2->getNumLODs() != 0)) {
        if ((flags >> 1 & 1) != 0) {
            param_2->calculateLOD(gpRender->getActiveCamera());
        }

        if (param_2->getActiveLODIndex() < 4) {
            uint32_t* piVar1 = (uint32_t*)param_2->getLOD(param_2->getActiveLODIndex()).pUnknown;
            if (*piVar1 == kHeightmapMagic) {
                Heightmap* pHmap = (Heightmap*)(piVar1 + 4); // + 0x10
                submitHeightmap(pHmap, param_2);
            } else {
                RenderObject* pObject = (RenderObject*)(piVar1 + 4); // + 0x10
                submitObject(pObject, param_2);
            }
        }
    }
}

void RenderScene::submitHeightmap(Heightmap * param_1, Instance * param_2)
{
    // FUN_005092f0
    OTDU_UNIMPLEMENTED;
}

void RenderScene::submitObject(RenderObject * param_1, Instance * param_2)
{
    // FUN_00509160
    OTDU_UNIMPLEMENTED;
}

bool RenderScene::isInstanceVisible(RenderObject * param_1, Instance * param_2)
{
    // FUN_005090b0
    Eigen::Vector4f local_40 = { 
        param_1->BoundingSphereOrigin[0], 
        param_1->BoundingSphereOrigin[1], 
        param_1->BoundingSphereOrigin[2],
        0.0f
    };

    Eigen::Vector4f local_30 = param_2->getModelMatrix() * local_40;
    local_30.w() = param_1->BoundingSphereRadius;

    const Eigen::Matrix4f& worldToCam = gpRender->getActiveCamera()->getWorldToCam();
    Eigen::Vector4f local_20 = worldToCam * local_30;

    int32_t iVar2 = gpRender->getActiveFrustum()->testBoundingSphere(local_20, param_1->BoundingSphereRadius * param_2->getBoundingScale() );
    return iVar2 != 0;
}
