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

// DAT_00eede20
static constexpr int32_t DAT_00eede20[5] = {
    0x10, 0x08, 0x04, 0x02, 0x01
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

        pVertexBuffer = gpRender->getRenderDevice()->createBuffer(&desc);
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
    : pIndexBuffer( nullptr )
    , pIndexBuffer2( nullptr )
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
    if (pIndexBuffer != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pIndexBuffer);
        pIndexBuffer = nullptr;
    }

    if (pIndexBuffer2 != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pIndexBuffer2);
        pIndexBuffer2 = nullptr;
    }
}

bool HeightmapTileRenderer::initialize()
{
    // FUN_00605d50
    // FUN_00605af0 (inlined)
    createAreaIndexBuffer();
    OTDU_UNIMPLEMENTED;
}

void HeightmapTileRenderer::createAreaIndexBuffer()
{
    // FUN_00604e60
    primitiveCount = 0x87d;

    GPUBufferDesc desc;
    desc.BufferType = GPUBufferDesc::Type::BT_IndexBuffer;
    desc.Size = 0x2200;
    desc.Stride = 4u;
    pIndexBuffer = gpRender->getRenderDevice()->createBuffer(&desc);

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

uint32_t FUN_006057e0(int32_t param_1, uint16_t* param_2)
{
    // FUN_006057e0  
    uint16_t* puVar5 = param_2;
    if (param_1 != 1) {           
        uint64_t uVar1 = 0x10ull / param_1;
        int32_t iVar7 = (int32_t)uVar1;

        int32_t iVar10 = param_1 - 1;
        uint32_t uVar4 = 1;
        if (1 < iVar10) {
            uint16_t local_24 = iVar7 * 0x104;
            uint16_t local_28 = iVar7 * 0x204;
            uint16_t local_2c = iVar7 * 0x305;
            uint16_t iVar15 = iVar7 * 0x203;
            uint16_t iVar12 = iVar7 * 0x304;
            uint16_t iVar9 = iVar7 * 0x102;

            int32_t local_c = 1;
            do {
                int16_t sVar3 = (int16_t)uVar1;
                *param_2 = iVar9 + sVar3;
                param_2[1] = iVar9;

                uint32_t uVar13 = 1;
                uint16_t* puVar8 = param_2 + 2;
                param_2 = puVar8;
                if (1 < iVar10) {
                    *param_2 = local_2c;
                    uint16_t iVar2 = iVar7 * 0x202;
                    uint16_t iVar11 = local_28;
                    uint16_t iVar14 = iVar15;
                    uint16_t* puVar6 = param_2;
                    uint16_t local_20 = iVar12;
                    do {
                        param_2 = puVar6;
                        puVar6 = param_2 + iVar7 * 0x101;
                        *puVar8 = iVar11;
                        puVar8[1] = iVar14;
                        puVar8[2] = *param_2;
                        puVar8[3] = local_20;
                        local_20 = local_20 + iVar2;
                        uVar13 = uVar13 + 2;
                        iVar11 = iVar11 + iVar2;
                        puVar8 = puVar8 + 4;
                        iVar14 = iVar14 + iVar2;
                        param_2 = puVar8;
                    } while (uVar13 < iVar10);
                }
                if ((local_c + 1) < iVar10) {
                    *param_2 = ((short)(uVar13 << 8) + uVar4 + (short)uVar13) * sVar3;
                    param_2[1] = (ushort)local_24;
                    param_2 = param_2 + 2;
                }
                uVar4 = uVar4 + 1;
                local_c = uVar4;
                local_2c += iVar7;
                local_28 = local_28 + iVar7;
                local_24 = local_24 + iVar7;
                iVar9 = iVar9 + iVar7;
                iVar12 = iVar12 + iVar7;
                iVar15 = iVar15 + iVar7;
            } while (local_c < iVar10);
        }

        auto ret_value = param_2 - puVar5;
        return ret_value >> 1;
    }
        
    *param_2 = 0x10;
    param_2[1] = 0x0;
    param_2[2] = 0x1020;
    param_2[3] = 0x1010;
    return 4;
}

uint32_t HeightmapTileRenderer::calculateNumIndexes()
{
    // FUN_006059e0                      
    uint16_t local_1800 [3070];

    uint32_t iVar7 = 0;
    for (int32_t iVar9 = 0; iVar9 < 5; iVar9++) {
        uint32_t uVar3 = FUN_006057e0(DAT_00eede20[iVar9], local_1800);
        iVar7 += uVar3;
    }
    
    for (int32_t local_1804 = 0; local_1804 < 5; local_1804++) {
        int32_t iVar1 = DAT_00eede20[local_1804];
        for (int32_t piVar8 = 0; piVar8 < 5; piVar8++) {
            int32_t iVar2 = DAT_00eede20[piVar8];
            
            // uVar4 = FUN_00605360((ushort)iVar1,(ushort)iVar2,local_1800,(undefined *)piVar9);
            // uVar5 = FUN_006055a0(iVar1,iVar2,local_1800);
            // uVar6 = FUN_00604fd0(iVar1,iVar2,local_1800);
            // piVar9 = (int32_t *)FUN_00605190(iVar1,iVar2,local_1800);
            // piVar8 = piVar8 + 1;
            // iVar7 = iVar7 + (uVar4 & 0xffff) + (uVar5 & 0xffff) + (uVar6 & 0xffff) +
            //         ((uint)piVar9 & 0xffff);
        }
    }
    OTDU_UNIMPLEMENTED;

    return iVar7;
}
