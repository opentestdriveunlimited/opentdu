#include "shared.h"
#include "scene_renderer.h"

#include "render_buckets.h"
#include "setup_node.h"
#include "instance.h"
#include "render_scene.h"
#include "material.h"
#include "gs_render.h"
#include "gs_render_helper.h"
#include "shaders/shader_constant_map.h"

SceneRenderer gSceneRenderer = {};

Eigen::Matrix4f gActiveTransformMatrix = Eigen::Matrix4f::Identity(); // DAT_00fe7370
Eigen::Matrix4f* gpActiveBonesArray = nullptr; // DAT_00fe89cc
static bool BOOL_00fe77a2 = false; // DAT_00fe77a2
static Eigen::Vector4f DAT_00fe7a70 = Eigen::Vector4f::Zero(); // DAT_00fe7a70
static bool BOOL_00fe777d = false; // DAT_00fe777d
static Eigen::Matrix4f gInvActiveTransformMatrix = Eigen::Matrix4f::Identity(); // DAT_00fe73b0
static Eigen::Vector4f DAT_00fac350 = Eigen::Vector4f::Zero(); // DAT_00fac350
static Eigen::Vector4f DAT_00fac260 = /* FUN_009e4fe0 */ { 1.0f, 0.0f, 0.0f, 0.0f }; // DAT_00fac260

// TODO: Should be owned by whatever system is reponsible for shader/PSO binding (not the scene renderer)
static ShaderConstantsMap* gpActiveConstantMap = nullptr; // DAT_00fadbcc

inline Eigen::Matrix4f& GetInvTransformMatrix()
{
    if (!BOOL_00fe777d) {
        gInvActiveTransformMatrix = gActiveTransformMatrix.inverse();
        BOOL_00fe777d = true;
    }
    return gInvActiveTransformMatrix;
}

inline void UploadConstant(ConstantMap* pMap, uint32_t offsetInMap, float* pScalars, uint32_t numVectors)
{
    uint32_t UVar1 = pMap->pConstants[offsetInMap];
    if (UVar1 != 0xffffffff) {
        eShaderType stage = (pMap->Type == 0) ? eShaderType::ST_Vertex : eShaderType::ST_Pixel;
        gpRender->getRenderDevice()->setFloatConstants(stage, pScalars, UVar1, numVectors);
    }
}

void FUN_005146b0(SceneRenderer* param_1)
{
    // FUN_005146b0
    OTDU_UNIMPLEMENTED;
}

void SceneRenderer::reset()
{
    ActiveShaderHashcode = 0ull;
    pActiveMaterial = nullptr;
    pActiveConstantMap0 = nullptr;
    pActiveConstantMap1 = nullptr;
    ActiveVSShaderInfos[0] = 0;
    ActiveVSShaderInfos[1] = 0;
    ActivePSShaderInfos[0] = 0;
    ActivePSShaderInfos[1] = 0;
    pActiveMaterialParameter = nullptr;
    NumVectors[0] = 0;
    NumVectors[1] = 0;
    NumMatrices[0] = 0;
    NumMatrices[1] = 0;
    BoundTextureProperties.clear();
}

void SceneRenderer::bindPrimitiveInstance(Primitive *param_1, TransformMatrixCommand *param_2)
{
    // FUN_005f1880
    pActiveInstance = param_2->pInstance;
    pActiveDrawList = param_2->pDrawList;
    pActiveTransform = param_2;
    
    param_2->pSetup->bind(0);

    if ((-1 < param_2->IsInstanciated) && (param_2->IsInstanciated < 2)) {
        bindInstance( nullptr, nullptr );
    }

    gInstanceRenderer.bind(param_1);
}

void SceneRenderer::bindInstance(TransformMatrixCommand *param_2, Instance *param_3)
{
    // FUN_005154d0
    gActiveTransformMatrix = (param_2 != nullptr) ? param_2->ModelMatrix : Eigen::Matrix4f::Identity();
    if (param_3 == nullptr) {
        gpActiveBonesArray = nullptr;
    } else {
        gpActiveBonesArray = param_3->getBones();
        gpRender->getRenderDevice()->setCullMode(
            pActiveMaterial->BackfaceCullingMode,
            (param_3->getFlags() >> 7 & 1) + (pActiveScene->getFlags() & 1) & 0x80000001
        );
        FUN_005146b0(this);
    }
    
    bindConstantsToGPU();
    SetupGraph::ExecuteCached(0);
    uploadLightsToGPU();
    bindMaterialParameters();
}

void SceneRenderer::bindConstantsToGPU()
{
    // FUN_005fc770
    for (int32_t iVar4 = 0; iVar4 < NumVectors[0]; iVar4++) {
        Eigen::Vector4f* pfVar1 = ConstantsVectors[iVar4].EntryPerStage[0].pCallback();
        gpRender->getRenderDevice()->setFloatConstants(eShaderType::ST_Vertex, pfVar1->data(), ConstantsVectors[iVar4].EntryPerStage[0].ConstantOffset, 1);
    }
    
    for (int32_t iVar4 = 0; iVar4 < NumMatrices[0]; iVar4++) {
        Eigen::Vector4f* pfVar1 = ConstantsMatrices[iVar4].EntryPerStage[0].pCallback();
        gpRender->getRenderDevice()->setFloatConstants(eShaderType::ST_Vertex, pfVar1->data(), ConstantsMatrices[iVar4].EntryPerStage[0].ConstantOffset, 4);
    }

    for (int32_t iVar4 = 0; iVar4 < NumVectors[1]; iVar4++) {
        Eigen::Vector4f* pfVar1 = ConstantsVectors[iVar4].EntryPerStage[1].pCallback();
        gpRender->getRenderDevice()->setFloatConstants(eShaderType::ST_Pixel, pfVar1->data(), ConstantsVectors[iVar4].EntryPerStage[1].ConstantOffset, 1);
    }
    
    for (int32_t iVar4 = 0; iVar4 < NumMatrices[1]; iVar4++) {
        Eigen::Vector4f* pfVar1 = ConstantsMatrices[iVar4].EntryPerStage[1].pCallback();
        gpRender->getRenderDevice()->setFloatConstants(eShaderType::ST_Pixel, pfVar1->data(), ConstantsMatrices[iVar4].EntryPerStage[1].ConstantOffset, 4);
    }
}

void SceneRenderer::uploadLightsToGPU()
{
    // FUN_00600c90
    if (pActiveMaterialParameter != nullptr) {
        uploadLightsToCbuffer();
    }
}

void SceneRenderer::uploadLightsToCbuffer()
{
    // FUN_00600aa0
    Light* local_24 = gBoundLights[0];
    uint8_t bVar6 = 0;

    uint32_t uVar4 = ActivePSShaderInfos[1];
    uint32_t uVar3 = ActivePSShaderInfos[0];
    uint32_t uVar2 = ActiveVSShaderInfos[1];
    uint32_t uVar1 = ActiveVSShaderInfos[0];

    uint32_t local_28 = 0;

    for (uint32_t local_c = 0; local_c < kMaxNumLightPerScene; local_c++) {
        Light* local_24 = gBoundLights[local_c];
        if (local_24 == nullptr) {
            break;
        }
        switch (local_24->Type) {
        case eLightType::LT_Ambient: {
            uint64_t lVar7 = (bVar6 << 0x8000);
            uint32_t uVar5 = (uint32_t)((lVar7 >> 0x20) & 0xffffffff);
            if (((uVar3 | uVar1) & (uint32_t)(lVar7 & 0xffffffff)) != 0 || ((uVar4 | uVar2) & uVar5) != 0) {
                uploadAmbientLight((AmbientLight*)local_24,local_28,((uint)lVar7 & uVar3) != 0 || (uVar5 & uVar4) != 0);
                local_28++;
                bVar6 += 4;
            }
            break;
        }
        case eLightType::LT_Directional: {
            uint64_t lVar7 = (bVar6 << 0x2000);
            uint32_t uVar5 = (uint32_t)((lVar7 >> 0x20) & 0xffffffff);
            if (((uVar3 | uVar1) & (uint32_t)(lVar7 & 0xffffffff)) != 0 || ((uVar4 | uVar2) & uVar5) != 0) {
                uploadDirectionalLight((DirectionalLight*)local_24,local_28,((uint)lVar7 & uVar3) != 0 || (uVar5 & uVar4) != 0);
                local_28++;
                bVar6 += 4;
            }
            break;
        }
        case eLightType::LT_Point: {
            uint64_t lVar7 = (bVar6 << 0x1000);
            uint32_t uVar5 = (uint32_t)((lVar7 >> 0x20) & 0xffffffff);
            if (((uVar3 | uVar1) & (uint32_t)(lVar7 & 0xffffffff)) != 0 || ((uVar4 | uVar2) & uVar5) != 0) {
                uploadPointLight((PointLight*)local_24,local_28,((uint)lVar7 & uVar3) != 0 || (uVar5 & uVar4) != 0);
                local_28++;
                bVar6 += 4;
            }
            break;
        }
        case eLightType::LT_Spot: {
            uint64_t lVar7 = (bVar6 << 0x4000);
            uint32_t uVar5 = (uint32_t)((lVar7 >> 0x20) & 0xffffffff);
            if (((uVar3 | uVar1) & (uint32_t)(lVar7 & 0xffffffff)) != 0 || ((uVar4 | uVar2) & uVar5) != 0) {
                uploadSpotLight((SpotLight*)local_24,local_28,((uint)lVar7 & uVar3) != 0 || (uVar5 & uVar4) != 0);
                local_28++;
                bVar6 += 4;
            }
            break;
        }
        default:
            OTDU_UNIMPLEMENTED;
            break;
        }
    }
}

void SceneRenderer::uploadDirectionalLight(DirectionalLight *param_1, int32_t param_2, bool param_3)
{
    // FUN_005fffd0
    Eigen::Vector4f local_90[3]= {
        param_1->Ambient.array() * pActiveMaterial->AmbientColor.array(),
        param_1->Diffuse.array() * pActiveMaterial->DiffuseColor.array(),
        param_1->Specular.array() * pActiveMaterial->SpecularColor.array()
    };
    applyInstanceColor(pActiveInstance, local_90[0], local_90[1], local_90[2]);

    Eigen::Vector4f local_b0 = Eigen::Vector4f(param_1->Direction.x(), param_1->Direction.y(), param_1->Direction.z(), 0.0f);
    Eigen::Vector4f local_60 = GetInvTransformMatrix() * local_b0;
    local_60.normalize();

    UploadConstant(pActiveConstantMap0, param_2 * 0x20 + 0x58, local_60.data(), 1);

    ConstantMap* peVar4 = (param_3) ? pActiveConstantMap1 : pActiveConstantMap0;
    UploadConstant(peVar4, (param_2 + 3) * 0x20, (float*)local_90, 3);
}

void SceneRenderer::uploadAmbientLight(AmbientLight *param_1, int32_t param_2, bool param_3)
{
    // FUN_005ffcc0
    Eigen::Vector4f local_20 = param_1->Color.array() * pActiveMaterial->AmbientColor.array();

    if (pActiveInstance != nullptr) {
        uint32_t uVar1 = pActiveInstance->getFlags();
        if ((uVar1 & 0x78) != 0 && (uVar1 & 8) != 0) {
            local_20.array() *= FUN_00513000().array();
        }
    }

    ConstantMap* pMap = (param_3) ? pActiveConstantMap1 : pActiveConstantMap0;
    UploadConstant(pMap, (param_2 + 0xea) * 4, local_20.data(), 1);
}

Eigen::Vector4f SceneRenderer::computeLightAttenuation(float param_1,float param_2)
{
    // FUN_005ffdc0
    // TODO: Might be inaccurate (function is never called at runtime)
    Eigen::Vector4f local_70 = DAT_00fac260 * param_2;
    Eigen::Vector4f local_80 = DAT_00fac260 * param_1;

    Eigen::Vector4f local_60 = GetInvTransformMatrix() * local_80;
    float fVar6 = local_60.x() * local_60.x() + local_60.y() * local_60.y();
    float fVar7 = local_60.y() * local_60.y() + local_60.x() * local_60.x();
    float fVar8 = local_60.z() * local_60.z() + local_60.w() * local_60.w();
    float fVar9 = local_60.w() * local_60.w() + local_60.z() * local_60.z();
    local_60.y() = fVar7 + fVar9;
    local_60.x() = fVar6 + fVar8;
    local_60.z() = fVar8 + fVar6;
    local_60.w() = fVar9 + fVar7;
    auto local_80_sqrt = local_60.array().sqrt();

    local_60 = GetInvTransformMatrix() * local_70;
    fVar6 = local_60.x() * local_60.x() + local_60.y() * local_60.y();
    fVar7 = local_60.y() * local_60.y() + local_60.x() * local_60.x();
    fVar8 = local_60.z() * local_60.z() + local_60.w() * local_60.w();
    fVar9 = local_60.w() * local_60.w() + local_60.z() * local_60.z();
    local_60.y() = fVar7 + fVar9;
    local_60.x() = fVar6 + fVar8;
    local_60.z() = fVar8 + fVar6;
    local_60.w() = fVar9 + fVar7;
    auto local_70_sqrt = local_60.array().sqrt();

    Eigen::Vector4f param_3 = {
        local_80_sqrt.x(),
        1e+06f,
        0.0f,
        0.0f
    };

    auto fVar2 = local_70_sqrt.x() - local_80_sqrt.x();
    if (1e-06f < abs(fVar2)) {
        param_3.y() = 1.0f / fVar2;
    }

    return param_3;
}

void SceneRenderer::uploadPointLight(PointLight *param_1, int32_t param_2, bool param_3)
{
    // FUN_006001f0
    Eigen::Vector4f local_90[3] = {
        param_1->Ambient.array() * pActiveMaterial->AmbientColor.array(),
        param_1->Diffuse.array() * pActiveMaterial->DiffuseColor.array(),
        param_1->Specular.array() * pActiveMaterial->SpecularColor.array()
    };
    applyInstanceColor(pActiveInstance, local_90[0], local_90[1], local_90[2]);

    Eigen::Vector4f local_60 = {
        DAT_00fac350.x(),
        DAT_00fac350.y(),
        0.0f,
        0.0f
    };

    if (param_1->Attenuated == 1) {
        local_60 = computeLightAttenuation(param_1->AttenuationCutoff, param_1->AttenuationDropoff);
    }

    Eigen::Vector4f local_70 = GetInvTransformMatrix() * param_1->Position;
    UploadConstant(pActiveConstantMap0, param_2 * 0x20 + 0x54, local_70.data(), 1);

    ConstantMap* pMap = (param_3) ? pActiveConstantMap1 : pActiveConstantMap0;
    UploadConstant(pMap, (param_2 + 3) * 0x20, local_90[0].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 100, local_90[1].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 0x68, local_90[2].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 0x5c, local_60.data(), 1);
}

void SceneRenderer::uploadSpotLight(SpotLight *param_1, int32_t param_2, bool param_3)
{
    // FUN_00600590
    Eigen::Vector4f local_90[3] = {
        param_1->Ambient.array() * pActiveMaterial->AmbientColor.array(),
        param_1->Diffuse.array() * pActiveMaterial->DiffuseColor.array(),
        param_1->Specular.array() * pActiveMaterial->SpecularColor.array()
    };
    applyInstanceColor(pActiveInstance, local_90[0], local_90[1], local_90[2]);

    Eigen::Vector4f local_60 = {
        DAT_00fac350.x(),
        DAT_00fac350.y(),
        0.0f,
        0.0f
    };

    if (param_1->Attenuated == 1) {
        local_60 = computeLightAttenuation(param_1->AttenuationCutoff, param_1->AttenuationDropoff);
    }

    if ((param_1->InnerAttenuation + -1) == 0) {
        float fVar8 = cos(0.5f * param_1->InnerAttenuationCutoff);
        local_60.z() = fVar8;
        fVar8 = cos((param_1->InnerAttenuationDropoff - param_1->InnerAttenuationCutoff) * 0.5f);
        local_60.w() = fVar8;
    }

    Eigen::Vector4f local_d0(
        param_1->Direction.x(), 
        param_1->Direction.y(),
        param_1->Direction.z(),
        0.0f
    );
    Eigen::Vector4f local_a0 = GetInvTransformMatrix() * local_d0;
    local_a0.normalize();

    Eigen::Vector4f local_b0(param_1->Position.x(), param_1->Position.y(), param_1->Position.z(), 0.0f);
    Eigen::Vector4f local_c0 = GetInvTransformMatrix() * local_b0;

    UploadConstant(pActiveConstantMap1, param_2 * 0x20 + 0x54, local_c0.data(), 1);
    UploadConstant(pActiveConstantMap1, param_2 * 0x20 + 0x58, local_a0.data(), 1);

    ConstantMap* pMap = param_3 ? pActiveConstantMap1 : pActiveConstantMap0;
    UploadConstant(pMap, (param_2 + 3) * 0x20, local_90[0].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 100, local_90[1].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 0x68, local_90[2].data(), 1);
    UploadConstant(pMap, param_2 * 0x20 + 0x5c, local_60.data(), 1);
}

Eigen::Vector4f& SceneRenderer::FUN_00513000()
{
    // FUN_00513000
    if (pActiveInstance != nullptr && !BOOL_00fe77a2) {
        const ColorRGBA& uVar1 = pActiveInstance->getColor().Color;

        DAT_00fe7a70.x() = (float)uVar1.R;
        DAT_00fe7a70.y() = (float)uVar1.G;
        DAT_00fe7a70.z() = (float)uVar1.B;
        DAT_00fe7a70.w() = (float)uVar1.A;

        DAT_00fe7a70 *= 0.003921569f; // (1.0f / 255.0f)
        BOOL_00fe77a2 = true;
    }

    return DAT_00fe7a70;
}

void SceneRenderer::applyInstanceColor(Instance* param_1, Eigen::Vector4f& ambient, Eigen::Vector4f& diffuse, Eigen::Vector4f& specular)
{
    // FUN_00600ca0
    if (param_1 != nullptr && ((param_1->getFlags() & 0x78) != 0)) {
        Eigen::Vector4f& pTVar4 = FUN_00513000();
        if ((param_1->getFlags() & 8) != 0) {
            ambient.array() *= pTVar4.array();
        }
        
        if ((param_1->getFlags() & 0x10) != 0) {
            diffuse.array() *= pTVar4.array();
        }

        if ((param_1->getFlags() & 0x20) != 0) {
            specular.array() *= pTVar4.array();
        }
    }
}

void SceneRenderer::bindMaterialParameters()
{
    // FUN_00513a50
    if (pActiveMaterial != nullptr) {
        gpActiveStaticParams = nullptr;
        gActiveNumStaticParams = 0ull;
        
        bool bVar2 = SetupGraph::ExecuteCached(1);
        if (bVar2) {
            MaterialParameter* peVar3 = pActiveMaterial->getParameterByIndex(0);
            uint64_t local_10 = (peVar3 != nullptr) ? peVar3->Hashcode : 0ull;

            if (0ull < gActiveNumStaticParams) {
                ShaderParameter* pSVar5 = gpActiveStaticParams;
                size_t uVar4 = gActiveNumStaticParams;
                do {
                    if (pSVar5->pMaterial == pActiveMaterial || pSVar5->ShaderHashcode == local_10) {
                        ConstantMap* pMap = &( (pSVar5->bBindToPixelShader) ? gpActiveConstantMap->FloatPS : gpActiveConstantMap->FloatVS );
                        UploadConstant(pMap, pSVar5->ParameterID * 4, (float *)pSVar5->pParameters, pSVar5->NumParameters);
                    }
                    
                    pSVar5++;
                    uVar4--;
                } while (uVar4 != 0ull);
            }
        }
    }
}
