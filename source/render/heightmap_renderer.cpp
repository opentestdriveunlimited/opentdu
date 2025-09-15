#include "shared.h"
#include "heightmap_renderer.h"

#include "gs_render.h"
#include "gs_render_helper.h"

// DAT_00f4766c
static VertexLayoutAttribute gHeightmapVertexDeclaration[] = {
    { 0, 0,     eVertexAttributeFormat::VAF_TEXCOORD_R32G32_SFLOAT,     0, 0x5, 0x0 },
    { 1, 0,     eVertexAttributeFormat::VAF_POSITION_R32_SFLOAT,        0, 0x5, 0x1 },  
    kVertexLayoutSentinel
};

HeightmapRenderer gHeightmapRenderer = {}; // DAT_00fe89d0
HeightmapTileRenderer gHeightmapTileRenderer = {}; // DAT_00fe8a38

HeightmapRenderer::HeightmapRenderer()
    : pVertexLayout(nullptr)
    , pVertexBuffer(nullptr)
{

}

HeightmapRenderer::~HeightmapRenderer()
{

}

void HeightmapRenderer::destroy()
{
    // FUN_006010e0
    if (pVertexLayout != nullptr) {
        gpRender->getRenderDevice()->destroyVertexLayout(pVertexLayout);
        pVertexLayout = nullptr;
    }
    
    if (pVertexBuffer != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pVertexBuffer);
        pVertexBuffer = nullptr;
    }
}

bool HeightmapRenderer::initialize()
{
    // FUN_00600f40
    if (pVertexLayout == nullptr) {
        pVertexLayout = gpRender->getRenderDevice()->createVertexLayout(gHeightmapVertexDeclaration);
        if (pVertexLayout == nullptr) {
            return false;
        }
    }

    if (pVertexBuffer == nullptr) {
        GPUBufferDesc desc;
        desc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
        desc.Size = 0x81008;

        pVertexBuffer = gpRender->getRenderDevice()->createBuffer(desc);
    }

    float* pfVar4 = (float*)gpRender->getRenderDevice()->lockBuffer(pVertexBuffer, 0, 0, true);
    if (pfVar4 != nullptr) {
        for (int32_t iVar3 = 0; iVar3 < 0x101; iVar3++) {
            for (int32_t iVar2 = 0; iVar2 < 0x101; iVar2++) {
                *pfVar4 = (float)iVar2;
                pfVar4[1] = (float)iVar3;

                pfVar4 += 2;
            }
        }
        gpRender->getRenderDevice()->unlockBuffer(pVertexBuffer);
    }

    return true;
}

HeightmapTileRenderer::HeightmapTileRenderer()
    : pVertexBuffer( nullptr )
    , pIndexBuffer( nullptr )
    , primitiveCount( 0 )
{
    // FUN_009e5cb0
    int32_t iVar1 = 0x7d;
    do {
        DrawCommand& psVar1 = drawCommands[iVar1];
        psVar1.PrimitiveCount = 0;
        psVar1.StartIndex = -1;
        psVar1.MinVertexIndex = 0;
        psVar1.NumVertices = 0;
        iVar1--;
    } while (iVar1 != 0);
}

HeightmapTileRenderer::~HeightmapTileRenderer()
{
}

void HeightmapTileRenderer::destroy()
{
    // FUN_006059b0
    if (pVertexBuffer != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pVertexBuffer);
        pVertexBuffer = nullptr;
    }

    if (pIndexBuffer != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pIndexBuffer);
        pIndexBuffer = nullptr;
    }
}

bool HeightmapTileRenderer::initialize()
{
    // FUN_00605d50
    // FUN_00605af0 (inlined)
    uploadIndexBuffer();
    OTDU_UNIMPLEMENTED;
}

void HeightmapTileRenderer::uploadIndexBuffer()
{
    // FUN_00604e60
    primitiveCount = 0x87d;

    GPUBufferDesc desc;
    desc.BufferType = GPUBufferDesc::Type::BT_IndexBuffer;
    desc.Size = 0x2200;
    desc.Stride = 4u;
    pIndexBuffer = gpRender->getRenderDevice()->createBuffer(desc);

    if (pIndexBuffer != nullptr) {
        uint32_t* pVStack_4 = (uint32_t*)gpRender->getRenderDevice()->lockBuffer( pIndexBuffer, 0, 0, true );
        if (pVStack_4 != nullptr) {
            bool bVar1 = false;
            uint32_t* puVar4 = nullptr;
            for (uint32_t uVar5 = 0; uVar5 < 0xf8f9; uVar5 += 0x808) {
                for (uint32_t uVar3 = 0; uVar3 < 0x101; uVar3 += 8) {
                    *pVStack_4 = uVar5 + uVar3;
                    uint32_t* puVar4 = pVStack_4 + 1;
                    if (bVar1) {
                        *puVar4 = *pVStack_4;
                        puVar4 = pVStack_4 + 2;
                        bVar1 = false;
                    }
                    *puVar4 = uVar5 + 0x808 + uVar3;
                    pVStack_4 = puVar4 + 1;
                }
                *pVStack_4 = *puVar4;
                pVStack_4 = puVar4 + 2;
                bVar1 = true;
            }

            gpRender->getRenderDevice()->unlockBuffer( pIndexBuffer );
        }
    }
}
