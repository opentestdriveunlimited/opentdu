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
#include "render/3dg.h"
#include "render/hiearchy_node.h"
#include "render/setup_node.h"

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
        submitInstance( peVar2 );
    }

    for (HiearchyNode* peVar5 : pDrawCommands->StaticHiearchies) {
        for (uint16_t iVar13 = peVar5->NumChildren + 1; iVar13 != 0; iVar13 = iVar13 + -1) {
            submitInstance( peVar5->pInstance );
            peVar5 = peVar5->pNext;
        }
    }

    for (DrawList* peVar2 : pDrawCommands->DynamicDrawLists) {
        submitDrawListToBucket(peVar2);
    }
    
    for (Instance* peVar2 : pDrawCommands->DynamicInstances) {
        submitInstance( peVar2 );
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
                    submitInstanceWithCustomMaterial(pObj, peVar6);
                }
            }
        }
    }
    
    for (auto& peVar10 : pDrawCommands->DynamicInstancesArrays) {
        for (Instance* peVar2 : *peVar10) {
            submitInstance(peVar2);
        }
    }
    
    for (HiearchyNode* peVar5 : pDrawCommands->DynamicHiearchies) {
        for (uint16_t iVar13 = peVar5->NumChildren + 1; iVar13 != 0; iVar13 = iVar13 + -1) {
            submitInstance( peVar5->pInstance );
            peVar5 = peVar5->pNext;
        }
    }
}

 // Default (or global) Light Setup?
static LightSetupNode DAT_00fe89f8; // DAT_00fe89f8

struct TransformMatrixCommand
{
    Eigen::Matrix4f                     ModelMatrix = Eigen::Matrix4f::Identity();
    uint32_t                            IsInstanciated = 0;
    uint32_t                            Unknown = 0;
    DrawList*                           pDrawList = nullptr;
    Instance*                           pInstance = nullptr;
    const SetupGraph*                   pSetup = nullptr;
};

struct DrawCommand
{
    uint32_t                            IsHeightmap = 0;
    uint32_t                            Unknown = 0;
    const Primitive*                    pPrimitive = nullptr;
    LOD*                                pActiveLOD = nullptr;
    std::list<TransformMatrixCommand>   Matrices;
};

struct DrawCommandMaterial
{
    uint32_t                    Unknown = 0;
    float                       ZSort = 0.0f;
    Material*                   pMaterial = nullptr;
    std::list<DrawCommand>    DrawCommands;
};

struct SceneSetupCommand
{
    LightSetupNode*                     pLightSetupNode;
    std::list<DrawCommandMaterial>      MaterialCommands;
};

struct RenderBucket 
{
    uint32_t Index;
    uint32_t Flags;
    uint32_t Unknown;
    uint32_t Unknown2;

    std::list<DrawCommand>        DrawCommands;
    std::list<SceneSetupCommand>  SceneSetupCommands;

    SceneSetupCommand* allocateSceneSetupCommand(LightSetupNode* param_2);
};

static RenderBucket* gRenderBuckets = nullptr; // DAT_00faf478

inline RenderBucket& GetBucket(const uint16_t param_1)
{
    return gRenderBuckets[param_1];
}

inline DrawCommandMaterial* InsertBasedOnDepth(SceneSetupCommand* sceneSetup, float z)
{
    // FUN_005f1900
    DrawCommandMaterial cmd;

    for (auto it = sceneSetup->MaterialCommands.begin(); it != sceneSetup->MaterialCommands.end(); it++) {
        if (it->ZSort > z) {
            auto insertIt = sceneSetup->MaterialCommands.insert(it, cmd);
            return &*insertIt;
        }
    }

    sceneSetup->MaterialCommands.push_back(cmd);
    return &sceneSetup->MaterialCommands.back();
}

void AddPrimToBucket(Material* param_1, DrawList* param_2, const Eigen::Matrix4f& param_3, const Primitive* param_4, SetupGraph* param_5, const uint32_t isHeightmap)
{
    // FUN_005f23c0
    RenderBucket& pBucket = GetBucket(param_1->OT);
    SceneSetupCommand* peVar2 = pBucket.allocateSceneSetupCommand(&DAT_00fe89f8);
    DrawCommandMaterial* puVar4 = param_1->pOTNode;
    if ((puVar4 == nullptr) || ((pBucket.Flags >> 2 & 1) != 0)) {
        peVar2->MaterialCommands.push_back({});
        puVar4 = &peVar2->MaterialCommands.back();
        
        puVar4->pMaterial = param_1;
        puVar4->Unknown = 0x0;
        param_1->pOTNode = puVar4;
    }

    DrawCommand* puVar6 = &puVar4->DrawCommands.back();
    puVar6->pActiveLOD = nullptr;
    puVar6->Unknown = 0;
    puVar6->IsHeightmap = isHeightmap;
    puVar6->pPrimitive = param_4;

    puVar6->Matrices.push_back({});
    TransformMatrixCommand* pfVar6 = &puVar6->Matrices.back();
    pfVar6->IsInstanciated = 1;
    pfVar6->Unknown = 0;
    pfVar6->ModelMatrix = param_3;
    pfVar6->pDrawList = param_2;
    pfVar6->pInstance = nullptr;
    pfVar6->pSetup = param_5;

    OTDU_UNIMPLEMENTED;
}

void AddPrimToBucket(Material* param_1, Instance* param_2, Primitive* param_3, const uint32_t isHeightmap, Eigen::Vector4f& boundingSphereAndRadius, LOD* param_6)
{
    // FUN_005f21b0
    RenderBucket& pBucket = GetBucket(param_1->OT);
    SceneSetupCommand* peVar2 = pBucket.allocateSceneSetupCommand(&DAT_00fe89f8);
    DrawCommandMaterial* peVar3 = nullptr;

    bool bBindNodeToMaterial = true;
    if ((pBucket.Flags >> 2 & 1) == 0) { // No Z sort?
        if ((pBucket.Flags & 3) == 0) { // Use shared/global cmd?
            peVar3 = param_1->pOTNode;
            if (peVar3 == nullptr) {
                peVar2->MaterialCommands.push_back({});
                peVar3 = &peVar2->MaterialCommands.back();
            } else {
                bBindNodeToMaterial = false;
            }
        } else {
            Eigen::Vector4f local_30 = { 
                boundingSphereAndRadius.x(),
                boundingSphereAndRadius.y(),
                boundingSphereAndRadius.z(),
                0.0f
            };
            
            Eigen::Vector4f local_40 = param_2->getModelMatrix() * local_30;
            local_30 = gActiveCamToWorld * local_40;
            
            float local_34 = local_30.z();
            if ((pBucket.Flags >> 1 & 1) != 0) { // Back to front/front to back sort flag?
                local_34 = local_30.z() * -1.0f;
            }
            peVar3 = InsertBasedOnDepth(peVar2, local_34);
            peVar3->ZSort = local_34;
        }
    } else {
        peVar2->MaterialCommands.push_back({});
        peVar3 = &peVar2->MaterialCommands.back();
    }

    if (bBindNodeToMaterial) {
        peVar3->pMaterial = param_1;
        peVar3->Unknown = 0x0;
        param_1->pOTNode = peVar3;
    }

    DrawCommand* puVar6 = param_3->pOTNode;
    if (puVar6 == nullptr || param_3->pOTNodeMaterial != peVar3) {
        peVar3->DrawCommands.push_back({});
        puVar6 = &peVar3->DrawCommands.back();
        puVar6->pActiveLOD = param_6;
        puVar6->Unknown = 0;
        puVar6->IsHeightmap = isHeightmap;
        puVar6->pPrimitive = param_3;
        param_3->pOTNode = puVar6;
        param_3->pOTNodeMaterial = peVar3;
    }

    puVar6->Matrices.push_back({});
    TransformMatrixCommand* pfVar6 = &puVar6->Matrices.back();
    pfVar6->IsInstanciated = 1;
    pfVar6->Unknown = 0;
    pfVar6->ModelMatrix = param_2->getModelMatrix();
    pfVar6->pDrawList = nullptr;
    pfVar6->pInstance = param_2;
    pfVar6->pSetup = param_2->getSetup();
}

SceneSetupCommand* RenderBucket::allocateSceneSetupCommand(LightSetupNode* param_2)
{
    // FUN_DAT_00fe89f8
    if (param_2 == nullptr) {
        param_2 = &DAT_00fe89f8;
    }

    SceneSetupCommand cmd;
    cmd.pLightSetupNode = param_2;
    SceneSetupCommands.push_back(cmd);
    return &SceneSetupCommands.back();
}

void RenderScene::submitDrawListToBucket(DrawList * param_2)
{
    // FUN_00508960
    for (uint32_t iVar6 = 0; iVar6 < param_2->getNumPrimitives(); iVar6++) {
        Material* peVar7 = param_2->getMaterialAtIndex( iVar6 );
        if (peVar7 == nullptr) {
            peVar7 = DAT_00f4751c; // TODO: I guess this is the fallback/pink material?
        }

        SetupGraph* peVar4 = param_2->getSetupAtIndex( iVar6 );
        if ( ( ( ( peVar7->FXFlags >> 3 ) & 1 ) != 0 )
        &&  gpRender->getActiveScene()->FUN_00508930(peVar7) ) {
            const Eigen::Matrix4f& modelMatrix = param_2->getMatrixAtIndex( iVar6 );
            const DrawPrimitive& primitive = param_2->getPrimitiveAtIndex( iVar6 );
            AddPrimToBucket(peVar7, param_2, modelMatrix, &primitive.Primitive, peVar4, 0);
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

        if (param_2->getActiveLODIndex() < kMaxNumLOD) {
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
 
bool RenderScene::FUN_00508930(Material * param_2) const
{
    // FUN_00508930
    return ((1ull << param_2->OT) & unknownMask) != 0;
}

void RenderScene::submitHeightmap(Heightmap * param_1, Instance * param_2)
{
    // FUN_005092f0
    if (((param_2->getFlags() & 1) == 0) || isInstanceVisible(param_1, param_2)) {
        Render2DM::HashTableEntry* peVar2 = (Render2DM::HashTableEntry*)param_2->getMaterialBank();
        Primitive* pPrimitive = (Primitive*)( (uint8_t*)param_1->PrimitiveSection + 0x10 );
        uint16_t uVar1 = pPrimitive->MaterialID;

        Material* peVar7 = DAT_00fe67dc.pMaterial;
        if (peVar2 != nullptr && uVar1 == 0xffff ) {
            RenderFile::Section* pMaterialSection = peVar2[uVar1 + 1].pMaterial;
            if ( pMaterialSection != nullptr ) {
                peVar7 = (Material*)(pMaterialSection + 1);
            } 
        }

        if ((peVar7->FXFlags >> 3 & 1) != 0
        &&  gpRender->getActiveScene()->FUN_00508930(peVar7) ) {
            AddPrimToBucket(peVar7, param_2, pPrimitive, 1u, param_1->BoundingSphere, param_2->getLODs());
        }
    }
}

void RenderScene::submitObject(RenderObject * param_1, Instance * param_2)
{
    // FUN_00509160
    if (((param_2->getFlags() & 1) == 0) || isInstanceVisible(param_1, param_2)) {
        RenderFile::Section* pPrimitiveSection = param_1->PrimitiveSection;
        if (pPrimitiveSection != nullptr) {
            Render2DM::HashTableEntry* peVar2 = (Render2DM::HashTableEntry*)param_2->getMaterialBank();
            Primitive* pPrimitive = (Primitive*)( (uint8_t*)pPrimitiveSection + 0x10 );
            for (uint32_t local_c = 0; local_c < param_1->NumPrimitives; local_c++) {
                uint16_t uVar1 = pPrimitive->MaterialID;

                Material* peVar7 = DAT_00fe67dc.pMaterial;
                if (peVar2 != nullptr && uVar1 == 0xffff ) {
                    RenderFile::Section* pMaterialSection = peVar2[uVar1 + 1].pMaterial;
                    if ( pMaterialSection != nullptr ) {
                        peVar7 = (Material*)(pMaterialSection + 1);
                    } 
                }

                if ((peVar7->FXFlags >> 3 & 1) != 0
                &&  gpRender->getActiveScene()->FUN_00508930(peVar7) ) {
                    AddPrimToBucket(peVar7, param_2, pPrimitive, 0u, param_1->BoundingSphere, nullptr);
                }
            }
        }
    }
}

void RenderScene::submitInstanceWithCustomMaterial(RenderObject * param_2, InstanceWithCustomMaterial * param_3)
{
    // FUN_00509250
    Instance* peVar1 = param_3->pInstance;
    if (((peVar1->getFlags() & 1) == 0) || isInstanceVisible(param_2, peVar1)) {
        Material* peVar2 = param_3->pMaterial;
        RenderFile::Section* peVar4 = param_2->PrimitiveSection;
        uint8_t* peVar4Bytes = (uint8_t*)peVar4;
        if (peVar4 != nullptr) {
            for (uint32_t i = 0; i < param_2->NumPrimitives; i++) {
                Primitive* pPrimitive = (Primitive*)( peVar4Bytes + 0x10 );

                if ((peVar2->FXFlags >> 3 & 1) != 0
                &&  gpRender->getActiveScene()->FUN_00508930(peVar2) ) {
                    AddPrimToBucket(peVar2, peVar1, pPrimitive, 0u, param_2->BoundingSphere, nullptr);
                }

                peVar4Bytes += peVar4->Size;
            }
        }
    }
}

bool RenderScene::isInstanceVisible(RenderObject * param_1, Instance * param_2)
{
    // FUN_005090b0
    Eigen::Vector4f local_40 = { 
        param_1->BoundingSphere.x(),
        param_1->BoundingSphere.y(),
        param_1->BoundingSphere.z(),
        0.0f
    };

    Eigen::Vector4f local_30 = param_2->getModelMatrix() * local_40;
    local_30.w() = param_1->BoundingSphere.w();

    const Eigen::Matrix4f& worldToCam = gpRender->getActiveCamera()->getWorldToCam();
    Eigen::Vector4f local_20 = worldToCam * local_30;

    int32_t iVar2 = gpRender->getActiveFrustum()->testBoundingSphere(local_20, param_1->BoundingSphere.w() * param_2->getBoundingScale() );
    return iVar2 != 0;
}
