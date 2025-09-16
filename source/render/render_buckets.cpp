#include "shared.h"
#include "render_buckets.h"

#include "render/instance.h"
#include "render/3dd.h"
#include "render/draw_list.h"
#include "render/2dm.h"
#include "render/material.h"
#include "render/3dg.h"
#include "render/hiearchy_node.h"
#include "render/setup_node.h"
#include "render/gs_render.h"
#include "render/scene_renderer.h"

 // Default (or global) Light Setup?
static LightSetupNode DAT_00fe89f8; // DAT_00fe89f8

RenderBucketData* gRenderBucketDatas = nullptr; // DAT_00faf478

inline RenderBucketData& GetBucket(const uint16_t param_1)
{
    return gRenderBucketDatas[param_1];
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

void AddPrimToBucket(Material* param_1, DrawList* param_2, const Eigen::Matrix4f& param_3, Primitive* param_4, SetupGraph* param_5, const uint32_t isHeightmap)
{
    // FUN_005f23c0
    RenderBucketData& pBucket = GetBucket(param_1->OT);
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

void AllocateRenderBuckets()
{
    OTDU_ASSERT(gRenderBucketDatas == nullptr);
    gRenderBucketDatas = new RenderBucketData[eRenderBucket::RB_Invalid + 1];
}

void FreeRenderBuckets()
{
    OTDU_ASSERT(gRenderBucketDatas != nullptr);
    delete[] gRenderBucketDatas;
    gRenderBucketDatas = nullptr;
}

void AddPrimToBucket(Material *param_1, Instance *param_2, Primitive *param_3, const uint32_t isHeightmap, Eigen::Vector4f &boundingSphereAndRadius, LOD *param_6)
{
    // FUN_005f21b0
    RenderBucketData& pBucket = GetBucket(param_1->OT);
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

void DrawCommand::execute(bool param_2, uint32_t param_3)
{
    // FUN_005f1d30
    if (param_2) {
        if (IsHeightmap == 0) {
            gpRender->bindPrimitive(pPrimitive, pActiveLOD);
        } else if (IsHeightmap == 1) {
            gpRender->bindHeightmap(pPrimitive, pActiveLOD);
        }

        if (!Matrices.empty()) {
            if ((param_3 & 4) != 0) {
                gSceneRenderer.bindPrimitiveInstance(pPrimitive, &Matrices.front());

                size_t uVar2 = Matrices.size();
                size_t iVar3 = 0ull;
                for (TransformMatrixCommand& puVar1 : Matrices) {
                    bool bVar1 = gInstanceRenderer.uploadInstance(puVar1.pInstance,&puVar1,iVar3 == uVar2 - 1);
                    if (!bVar1) {
                        gpRender->drawIndexedPrimitive(pPrimitive);
                    }
                    iVar3++;
                }

                gInstanceRenderer.unbind();
                SetupGraph::Unbind(1);
            } else {
                for (TransformMatrixCommand& cmd : Matrices) {
                    cmd.execute(IsHeightmap, pPrimitive, pActiveLOD);
                }
            }
        }
    } 
    Matrices.clear();
}

void DrawCommandMaterial::execute()
{
    // FUN_005f1e60
    gSceneRenderer.reset();
    gpRender->bindMaterial(pMaterial);
    pMaterial->bindConstants();
    
    for (DrawCommand& drawCmd : DrawCommands) {
        drawCmd.execute(pMaterial->DepthTest, pMaterial->FXFlags);

        drawCmd.pPrimitive->pOTNode = nullptr;
        drawCmd.pPrimitive->pOTNodeMaterial = nullptr;
    }
    DrawCommands.clear();
}

void SceneSetupCommand::execute()
{
    // FUN_005f2150
    pLightSetupNode->execute(); // FUN_00513950 (inlined)
    
    for (DrawCommandMaterial& materialCmd : MaterialCommands) {
        materialCmd.execute();

        materialCmd.pMaterial->pOTNode = nullptr;
    }
    MaterialCommands.clear();
}

SceneSetupCommand* RenderBucketData::allocateSceneSetupCommand(LightSetupNode* param_2)
{
    // FUN_00fe89f8
    if (param_2 == nullptr) {
        param_2 = &DAT_00fe89f8;
    }

    SceneSetupCommand cmd;
    cmd.pLightSetupNode = param_2;
    SceneSetupCommands.push_back(cmd);
    return &SceneSetupCommands.back();
}

void TransformMatrixCommand::execute(uint32_t isHeightmap, Primitive *param_3, LOD *param_4)
{
    // FUN_005f17d0
    gSceneRenderer.pActiveInstance = pInstance;
    gSceneRenderer.pActiveDrawList = pDrawList;
    gSceneRenderer.pActiveTransform = this;
    
    pSetup->bind(1);
    if (0xffffffff < IsInstanciated && IsInstanciated < 2) {
        gSceneRenderer.bindInstance(this, this->pInstance);
    }

    if (isHeightmap == 0) {
        gpRender->drawIndexedPrimitive(param_3);
    } else if (isHeightmap == 1) {
        gpRender->drawHeightmap(param_3, param_4);
    }

    SetupGraph::Unbind(1);
}
