#include "shared.h"
#include "mng_car_alpha.h"

#include "render/material.h"
#include "render/render_target.h"
#include "render/gs_render.h"

MngCarAlpha* gpMngCarAlpha = nullptr;

MngCarAlpha::MngCarAlpha()
    : Manager()
    , bEnabled( false )
    , stencilMask( 0xffffffff )
    , numElements( 0 )
{
    OTDU_ASSERT( gpMngCarAlpha == nullptr );
    gpMngCarAlpha = this;
}

MngCarAlpha::~MngCarAlpha()
{
    bEnabled = false;
    gpMngCarAlpha = nullptr;
}

bool MngCarAlpha::initialize( TestDriveGameInstance* )
{
    bEnabled = true;
    stencilMask = gpRender->getFreeStencilMask();

    int32_t iVar3 = 4;
    do {
        uint32_t iVar4 = gpRender->getFreeStencilMask();
        if (iVar4 == 0xffffffff) {
            bEnabled = false;
        }
        iVar3--;
    } while (iVar3 != 0);

    uint32_t uVar5 = 0;
    do {
        if (!bEnabled) {
            break;
        } 
        bool bVar2 = initializeElement(uVar5);
        uVar5 = uVar5 + 1;
        bEnabled = bVar2;
    } while (uVar5 < 31);

    bind2DBsCallback = std::bind(&MngCarAlpha::bindBitmapsToScene, this);
    if (bind2DBsCallback != nullptr) {
        gpRender->registerMngCallback(bind2DBsCallback);
    }

    return true;
}

void MngCarAlpha::tick( float, float )
{

}

void MngCarAlpha::terminate()
{

}

bool MngCarAlpha::bindRenderTargetToElements()
{
    // FUN_006b8ff0
    for (uint32_t iVar4 = 0; iVar4 < kNumAlphaElements; iVar4++) {
        if (elements[iVar4].pMaterial != nullptr) {
            MaterialParameter* peVar1 = elements[iVar4].pMaterial->getParameterByIndex(0);
            MaterialShaderParameterArray* iVar2 = (MaterialShaderParameterArray*)peVar1->getLayer(0);

            RenderTarget* pRT = gpRender->getRenderPass<eRenderPass::RP_NearCarAlpha>().pFramebufferAttachments->pAttachments[0];
            Render2DB* p2DB = pRT->getBound2DB();
            iVar2->pParameters = p2DB->getBitmapSection();
        }
    }
    return true;
}

bool MngCarAlpha::initializeElement(uint32_t param_1)
{
    // FUN_006b9140
    AlphaElement& pCVar2 = elements[param_1];
    pCVar2.Mask = param_1 + 1 << (stencilMask & 0x1f);

    uint32_t uVar5 = Render2DM::CalcSize(3,0x30);
    pCVar2.pAlpha2DMBuffer = TestDrive::Alloc(uVar5);
    if (pCVar2.pAlpha2DMBuffer == nullptr) {
        return false;
    }
    pCVar2.pMaterial = Render2DM::Create(pCVar2.pAlpha2DMBuffer, 0xca9a1fa000000000,1,3,0x30,0);
    MaterialShaderParameterArray* pHVar8 = (MaterialShaderParameterArray*)pCVar2.pMaterial->getParameterByIndex(0);
    pCVar2.pMaterial->getParameterByIndex(0)->Flags.NumUVMaps = '\0';
    pCVar2.pMaterial->SrcBlend = '\x05';
    pCVar2.pMaterial->DstBlend = '\x06';
    pCVar2.pMaterial->BlendOP = '\x01';
    pCVar2.pMaterial->DepthTest = '\x02';
    pCVar2.pMaterial->DepthWrite = '\x02';
    pCVar2.pMaterial->BackfaceCullingMode = '\x03';

    RenderTarget* pRT = gpRender->getRenderPass<eRenderPass::RP_NearCarAlpha>().pFramebufferAttachments->pAttachments[0];
    Render2DB* peVar8 = pRT->getBound2DB();

    MaterialLayer* iVar7 = pCVar2.pMaterial->getParameterByIndex(0)->getLayer(0);
    iVar7->NumTextures = 1;
    iVar7->pLayerTextures[0].Hashcode = 0x0000000100000000;
    iVar7->pLayerTextures[0].pTextureSection = peVar8->getBitmapSection();
    iVar7->pLayerTextures[0].SamplerAddress[0] = '\x02';
    iVar7->pLayerTextures[0].SamplerAddress[1] = '\x02';
    iVar7->pLayerTextures[0].MinMagFilter = '\x01';
    iVar7->pLayerTextures[0].MipFilter = '\x01';

    uint32_t uVar4 = pHVar8->NumParameters;
    
    MaterialShaderParameter* peVar10 = nullptr;
    float* pTVar11 = nullptr;
    if (uVar4 > 0) {
        peVar10 = pHVar8->getParameterByIndex( 0 );
        if (uVar4 - 1 < uVar4) {
            pTVar11 = (float*)pHVar8->getLastParameter();
            pTVar11 += 4; // += 0x10 bytes
        }
    }
    pCVar2.pShaderConstantsVS = pTVar11;

    if (peVar10 != nullptr) {
        peVar10->Size = 0x20;
        peVar10->bBindToVS = 1;
        peVar10->Format = 2;
        peVar10->Type = 0x3e;
    }

    MaterialShaderParameter* peVar9 = nullptr;
    if (uVar4 >= 2) {
        peVar9 = pHVar8->getParameterByIndex( 1 );
        if (uVar4 - 1 < uVar4) {
            pTVar11 = (float*)pHVar8->getLastParameter();
            pTVar11 += 12; // += 0x10 + 0x20 (peVar10->Size)
        }
    }
    pCVar2.pShaderConstantsPS = pTVar11;

    if (peVar9 != nullptr) {
        peVar9->Size = 0x10;
        peVar9->bBindToVS = 0;
        peVar9->Format = 2;
        peVar9->Type = 0x3e;
    }

    pCVar2.Alpha2DM.initialize(pCVar2.pAlpha2DMBuffer);
    pCVar2.pMaterial->StencilEnable = '\x01';
    pCVar2.pMaterial->StencilFail = '\0';
    pCVar2.pMaterial->StencilZFail = '\0';
    pCVar2.pMaterial->StencilPass = '\0';
    pCVar2.pMaterial->StencilFunc = '\x02';
    pCVar2.pMaterial->StencilRef = pCVar2.Mask;
    pCVar2.pMaterial->StencilMask = 0;
    pCVar2.pMaterial->StencilWriteMask = 0;

    DrawStreams streams;
    streams.Index = 1;
    streams.Custom = 0x12;
    
    bool bVar3 = pCVar2.ComposingDrawList.initialize(1, 4, 0, false, &streams, "CarAlphaComp", false);
    if (bVar3) {
        pCVar2.ComposingDrawList.reset();
        pCVar2.ComposingDrawList.setIdentityTransform();
        pCVar2.ComposingDrawList.setActiveMaterial(pCVar2.pMaterial);
        bVar3 = pCVar2.ComposingDrawList.beginPrimitive( ePrimitiveType::PT_TriangleStrip, 4 );
        if (bVar3) {
            pCVar2.ComposingDrawList.pushVertex(0.0f,0.0f,0.0f);
            pCVar2.ComposingDrawList.pushVertex(0.0f,1.0f,0.0f);
            pCVar2.ComposingDrawList.pushVertex(1.0f,0.0f,0.0f);
            pCVar2.ComposingDrawList.pushVertex(1.0f,1.0f,0.0f);

            pCVar2.ComposingDrawList.commitPrimitive();
        }
        return true;
    }
    return false;
}

void MngCarAlpha::registerMaterial(uint32_t param_2, Material *param_3, bool param_4)
{
    // FUN_006b90c0
    if (((param_3 != nullptr) && (param_2 < 0x1f)) && (elements[param_2].pListener != nullptr)) {
        uint32_t uVar1 = 0x1f << ((uint8_t)stencilMask & 0x1f);
        param_3->StencilEnable = '\x01';
        param_3->StencilFail = '\0';
        param_3->StencilZFail = '\0';

        uint32_t uVar2 = param_3->StencilRef & ~uVar1;
        param_3->StencilRef = uVar2;
        param_3->StencilPass = '\x02';
        param_3->StencilFunc = '\a';
        param_3->StencilRef = elements[param_2].Mask | uVar2;

        uVar2 = param_3->StencilWriteMask & ~uVar1;
        param_3->StencilMask = param_3->StencilMask | uVar1;
        param_3->StencilWriteMask = uVar2;

        if (param_4) {
            param_3->StencilWriteMask = uVar2 | uVar1;
        }
    }
}

void MngCarAlpha::freeElement(uint32_t param_1)
{
    elements[param_1].pListener = nullptr;
    numElements--;
}

bool MngCarAlpha::bindBitmapsToScene()
{
    // FUN_00432620
    // FUN_006b8ff0 (inlined)
    for (AlphaElement& ppeVar5 : elements) {
        MaterialParameter* peVar1 = ppeVar5.pMaterial->getParameterByIndex(0);
        MaterialLayer* peVar2 = peVar1->getLayer(0);

        RenderPass& nearCarAlphaPass = gpRender->getRenderPass<eRenderPass::RP_NearCarAlpha>();
        Render2DB* peVar3 = nearCarAlphaPass.pFramebufferAttachments->pAttachments[0]->getBound2DB();

        peVar2->pLayerTextures[0].pTextureSection = peVar3->getBitmapSection();
    }
}
