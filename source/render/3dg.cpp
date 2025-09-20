#include "shared.h"
#include "3dg.h"

#include "render/gs_render_helper.h"
#include "render/gs_render.h"
#include "render/geometry_buffer.h"

static constexpr uint32_t kGeometryArrayMagic   =  0x414f4547; // GEOA (GEOmetry Array)

static uint32_t gNumPrimitives = 0u; // DAT_00fe88a0
RenderPool<Primitive> gPrimitivePool; // DAT_00fad0d0

// TODO: Move this to a separate header?
inline void UnpackRGB10ToFloat32Vertex(int32_t iVar5, float* pfVar1)
{
    int32_t iVar2 = (iVar5 << 0x16) >> 0x16;
    int32_t iVar6 = (iVar5 << 0xc) >> 0x16;
    iVar5 = iVar5 * 4 >> 0x16;
    pfVar1[-2] = (float)((uint32_t)(iVar2 == -0x200) + iVar2) * 0.0019569471f;
    pfVar1[-1] = (float)((uint32_t)(iVar6 == -0x200) + iVar6) * 0.0019569471f;
    *pfVar1    = (float)((uint32_t)(iVar5 == -0x200) + iVar5) * 0.0019569471f;
}

inline void UnpackRGB10ToFloat16Vertex(int32_t iVar4, int16_t* puVar5)
{
    // TODO: pcode cleanup
    int32_t iVar1 = (iVar4 << 0x16) >> 0x16;
    int32_t iVar2 = (iVar4 << 0xc) >> 0x16;
    iVar4 = iVar4 * 4 >> 0x16;
    puVar5[1] = 0x7fff;
    puVar5[-2] = (int16_t)(int32_t)((float)(int32_t)((uint32_t)(iVar1 == -0x200) + iVar1) * 0.0019569471 *
                                  32767.0 + 0.5);
    puVar5[-1] = (int16_t)(int32_t)((float)(int32_t)((uint32_t)(iVar2 == -0x200) + iVar2) * 0.0019569471f *
                    32767.0f + 0.5f);
    *puVar5 = (int16_t)(int32_t)((float)(int32_t)((uint32_t)(iVar4 == -0x200) + iVar4) * 0.0019569471f * 32767.0f
                + 0.5f);
}

static bool UnpackRGB10ToFloat32(int8_t *param_1, int8_t *param_2, uint32_t param_3)
{
    // FUN_00605d90
    uint32_t uVar4 = 0;
    if (3 < param_3) {
        int32_t* piVar7 = (int32_t *)(param_2 + 8);
        int32_t iVar3 = (param_3 - 4 >> 2) + 1;
        uVar4 = iVar3 * 4;
        float* pfVar1 = (float *)(param_1 + 8);
        do {
            UnpackRGB10ToFloat32Vertex(piVar7[-2], &pfVar1[-2]);
            UnpackRGB10ToFloat32Vertex(piVar7[-1], &pfVar1[1]);
            UnpackRGB10ToFloat32Vertex(piVar7[0], &pfVar1[4]);
            UnpackRGB10ToFloat32Vertex(piVar7[1], &pfVar1[7]);

            piVar7 += 4;
            pfVar1 += 0xc;
            iVar3--;
        } while (iVar3 != 0);
    }
    if (uVar4 < param_3) {
        float* pfVar1 = (float *)(param_1 + uVar4 * 0xc + 8);
        do {
            int32_t iVar3 = *(int32_t*)(param_2 + uVar4 * 4);
            UnpackRGB10ToFloat32Vertex(iVar3, &pfVar1[-2]);
            uVar4++;
            pfVar1 += 3;
        } while (uVar4 < param_3);
    }
    return true;
}

static bool UnpackRGB10ToFloat16(int8_t *param_1, int32_t *param_2, uint32_t param_3)
{
    // FUN_00606030
    uint32_t uVar3 = 0;
    if (param_3 != 0) {
        int16_t* puVar5 = (int16_t *)(param_1 + 4);
        do {
            int32_t iVar4 = param_2[uVar3];
            UnpackRGB10ToFloat16Vertex(iVar4, puVar5);
            uVar3++;
            puVar5 += 4;
        } while (uVar3 < param_3);
    }
    return true;
}

static bool ConvertVertices(int8_t *param_1, eVertexAttributeFormat dstFormat, int8_t *param_3, eVertexAttributeFormat srcFormat, uint32_t param_5)
{
    // FUN_006061e0
    if (srcFormat == eVertexAttributeFormat::VAF_TANGENT_A2B10G10R10_SNORM_PACK32
    || srcFormat == eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32) {
        if (dstFormat == eVertexAttributeFormat::VAF_TANGENT_R32G32B32_SFLOAT
         || dstFormat == eVertexAttributeFormat::VAF_NORMAL_R32G32B32_SFLOAT) {
            return UnpackRGB10ToFloat32(param_1,param_3,param_5);
        } else if (dstFormat == eVertexAttributeFormat::VAF_TANGENT_R16G16B16A16_SNORM
                || dstFormat == eVertexAttributeFormat::VAF_NORMAL_R16G16B16A16_SNORM) {
            return UnpackRGB10ToFloat16(param_1,(int32_t*)param_3,param_5);
        }
    }

    return false;
}

Render3DG::Render3DG()
     : RenderFile()
     , pGeometryArray( nullptr )
{

}

Render3DG::~Render3DG()
{

}

bool Render3DG::parseSection(RenderFile::Section *pSection)
{
    // FUN_0050a2a0
    switch (pSection->Type) {
    case kGeometryArrayMagic:
        pGeometryArray = pSection;
        return true;

    case kPrimitiveMagic:
        return parsePrimitiveSection(pSection);

    default:
        return true;
    };
}

void Render3DG::unparseSection(RenderFile::Section *pSection)
{
    // FUN_0050a2d0
    if (pSection->Type == kPrimitiveMagic) {
        unstreamPrimitive(pSection);
    }
}

bool Render3DG::UploadPrimitiveToGPU(Primitive *param_1, LOD* param_2)
{
    // FUN_005fe4a0
    PrimtiveVertexAttributes local_38 = param_1->VertexStreams;                                      
    int8_t* peVar1Header = param_1->pVertexBuffer;
    uint32_t uVar1 = local_38.adjustAttributesFormat();
    int8_t* peVar3 = nullptr;
    if (uVar1 != 0) {  
        uint32_t uVar2 = 0;                     
        if (uVar1 == 2) {
            uVar2 = local_38.calculateVertexBufferSize(param_1->NumVertex);
            peVar3 = (int8_t*)TestDrive::Alloc(uVar2 + 0x38);
            WritePrimitiveHeader(peVar3, local_38, (peVar1Header + 0x40), param_1->VertexStreams, param_1->NumVertex);
            WriteAttributeOffsets(param_1->pVertexAttributeOffsets, local_38, param_1->NumVertex);

            int8_t* pVertexAttributesOffset = peVar3 + uVar2;
            memcpy(pVertexAttributesOffset, &param_1->VertexStreams, sizeof(PrimtiveVertexAttributes));
            param_1->VertexStreams = local_38;
        } else {
            uVar2 = param_1->VertexStreams.calculateVertexBufferSize(param_1->NumVertex);
            peVar3 = peVar1Header + 0x40;
        }

        GPUBufferDesc vboDesc;
        vboDesc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
        vboDesc.bDynamic = false;
        vboDesc.Size = uVar2;
                
        uint32_t uVar8 = uVar2;
        GPUBuffer* pVertexBuffer = gpRender->getRenderDevice()->createBuffer(&vboDesc);
        if (pVertexBuffer != nullptr) {
            GeometryBuffer* pStreamingHeader = (GeometryBuffer*)(peVar1Header + 0x10);
            pStreamingHeader->pGPUBuffer = pVertexBuffer;
            pStreamingHeader->pCPUBuffer = peVar3;
            pStreamingHeader->ByteSize = uVar2;
            pStreamingHeader->LockStart = 0;
            pStreamingHeader->LockEnd = uVar2;
            pStreamingHeader->pCPUBufferCopy = nullptr;
            pStreamingHeader->DataOffset = 0;
            pStreamingHeader->bLocked = true;
            pStreamingHeader->Flags = 0x10; // TODO: Check constness at runtime
            pStreamingHeader->pGPUBufferCopy = nullptr;
            
            peVar3 = param_1->pIndexBuffer;
            uVar2 = param_1->NumIndex;
            uint32_t eStack_18 = 0;
            if (param_1->IndexType == 0) {
                uVar2 = uVar2 * 4;
                eStack_18 = sizeof( uint32_t );
            } else if (param_1->IndexType == 1) {
                uVar2 = uVar2 * 2;
                eStack_18 = sizeof( uint16_t );
            }
            
            GPUBufferDesc iboDesc;
            iboDesc.BufferType = GPUBufferDesc::Type::BT_IndexBuffer;
            iboDesc.bDynamic = false;
            iboDesc.Size = uVar2;
            iboDesc.Stride = eStack_18;

            GPUBuffer* pIndexBuffer = gpRender->getRenderDevice()->createBuffer(&iboDesc);
            if (pIndexBuffer != nullptr) {
                GeometryBuffer* pStreamingHeader = (GeometryBuffer*)(peVar3 + 0x10);
                pStreamingHeader->pGPUBuffer = pIndexBuffer;
                pStreamingHeader->pCPUBuffer = (peVar3 + 0x40);
                pStreamingHeader->ByteSize = uVar2;
                pStreamingHeader->LockStart = 0;
                pStreamingHeader->LockEnd = uVar2;
                pStreamingHeader->pCPUBufferCopy = nullptr;
                pStreamingHeader->DataOffset = 0;
                pStreamingHeader->bLocked = true;
                pStreamingHeader->Flags = 0x20; // TODO: Check constness at runtime
                pStreamingHeader->pGPUBufferCopy = nullptr;

                gNumPrimitives++;
                CreateVertexDeclaration(param_1);
                param_1->bUploaded = true;
                return true;
            }
        }
    }
    
    return false;
}

bool Render3DG::UploadHeightmapToGPU(Primitive *param_1, LOD *param_2)
{
    // FUN_005fee00
    OTDU_UNIMPLEMENTED;
    return false;
}

void Render3DG::CreateVertexDeclaration(Primitive *param_1)
{
    // FUN_005fdbb0
    if (param_1->VertexStreams.getNumAttributes() < 0x11) {
        OTDU_UNIMPLEMENTED;
    }
}

void Render3DG::Upload(Primitive *param_1)
{
    // FUN_005122f0
    if (param_1 == nullptr) {
        return;
    }

    if (param_1->Type == ePrimitiveType::PT_HMap) {
        UploadHeightmapToGPU(param_1, nullptr);
    } else {
        UploadPrimitiveToGPU(param_1, nullptr);
    }
}

void Render3DG::ReleaseUploadedPrimitive(Primitive *param_1)
{
    // FUN_005122d0
    if (param_1 == nullptr) {
        return;
    }

    if (param_1->Type == ePrimitiveType::PT_HMap) {
        UnstreamHeightmap(param_1);
    } else {
        UnstreamGeometry(param_1);
    }
}

bool Render3DG::parsePrimitiveSection(RenderFile::Section *pSection)
{
    // FUN_00512280
    if (pSection == nullptr) {
        return false;
    }

    Primitive* pPrimitive = (Primitive*)pSection;
    pPrimitive->bUploaded = false;
    gPrimitivePool.addToPool(pPrimitive);

    return true;
}

void Render3DG::unstreamPrimitive(RenderFile::Section *pSection)
{
    // FUN_005122a0
    Primitive* param_1 = (Primitive*)(pSection + 1);
    gPrimitivePool.removeFromPool(param_1);
    if (param_1->Type == ePrimitiveType::PT_HMap) {
        UnstreamHeightmap(param_1);
    } else {
        UnstreamGeometry(param_1);
    }
}

void Render3DG::UnstreamHeightmap(Primitive *param_1)
{
    // FUN_005ff010
    if (!param_1->bUploaded) {
        return;
    }
    param_1->bUploaded = false;
    OTDU_UNIMPLEMENTED; 
    // GPUBuffer** piVar2 = FUN_005fed10(param_1);
    // if (*piVar2 != nullptr) {
    //     gpRender->getRenderDevice()->destroyBuffer(*piVar2);
    // }
    // *piVar2 = nullptr;
}

void Render3DG::UnstreamGeometry(Primitive *param_1)
{
    // FUN_005fe670
    OTDU_UNIMPLEMENTED;
}

bool Render3DG::WritePrimitiveHeader(int8_t *pPatchedHeader, PrimtiveVertexAttributes &patchedVA, int8_t *pOriginalHeader, const PrimtiveVertexAttributes &originalVA, uint32_t numVertex)
{
    // FUN_006065c0
#define OTDU_WRITE_VERTEX_ATTRIBUTE( attrib )\
    if (originalVA.attrib.Number != '\0') {\
        if (!Render3DG::WritePatchedVertices(pPatchedHeader, patchedVA.attrib, pOriginalHeader, originalVA.attrib, numVertex)) {\
            return false;\
        }\
    }\
    
    OTDU_WRITE_VERTEX_ATTRIBUTE( Position );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Normal );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Diffuse );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Specular );
    OTDU_WRITE_VERTEX_ATTRIBUTE( UV );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Tangent );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Binormal );
    OTDU_WRITE_VERTEX_ATTRIBUTE( Bones );
    OTDU_WRITE_VERTEX_ATTRIBUTE( BoneWeight );

    return true;
#undef OTDU_WRITE_VERTEX_ATTRIBUTE
}

bool Render3DG::WritePatchedVertices(int8_t *pPatchedHeader, PrimtiveVertexAttributes::Stream &patchedAttribute, int8_t *pOriginalHeader, const PrimtiveVertexAttributes::Stream &originalAttribute, uint32_t numVertex)
{
    // FUN_006064e0
    eVertexAttributeFormat srcFormat = originalAttribute.Format;
    uint8_t bVar1 = originalAttribute.Number;
    uint32_t iVar4 = GetVertexAttributeSize(srcFormat);
    
    eVertexAttributeFormat dstFormat = patchedAttribute.Format;
    uint32_t uVar6 = iVar4 * bVar1 * numVertex;
    uint32_t iVar3 = GetVertexAttributeSize(dstFormat);
    iVar4 = iVar3 * patchedAttribute.Number * numVertex;

    if (srcFormat == dstFormat) {
        // TODO: generated pCode had some messy way to memcpy (dwords first then bytes). 
        // memcpy should achieve the same thing if the assert below is always valid
        OTDU_ASSERT(iVar4 == uVar6);
        memcpy(pPatchedHeader, pOriginalHeader, uVar6);

        pOriginalHeader += uVar6;
        pPatchedHeader += iVar4;
        return true;
    }

    bool bVar2 = ConvertVertices(pPatchedHeader,dstFormat,pOriginalHeader,srcFormat,bVar1 * numVertex);
    if (!bVar2) {
        return bVar2;
    }

    pOriginalHeader += uVar6;
    pPatchedHeader += iVar4;
    return true;
}

inline void WriteAttributeOffset(const PrimtiveVertexAttributes::Stream& attrib, const uint32_t numVertex, int8_t *pOffsetTable, int32_t& iVar4, int32_t& iVar5)
{
    if (attrib.Number == '\0') {
        return;
    }

    int32_t iVar1 = GetVertexAttributeSize(attrib.Format);
    for (int32_t iVar3 = 0; iVar3 < attrib.Number; iVar3++) {
        int32_t* pOffset = (int32_t*)(pOffsetTable + iVar5 * 4);
        *pOffset = iVar4;

        iVar4 += iVar1 * numVertex;
        iVar5++;
    }
}

void Render3DG::WriteAttributeOffsets(int8_t *pOffsetTable, PrimtiveVertexAttributes &patchedVA, uint32_t numVertex)
{
    // FUN_005fe2a0
    int32_t iVar4 = 0;
    int32_t iVar5 = 0;

    WriteAttributeOffset(patchedVA.Position, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Normal, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Diffuse, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Specular, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.UV, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Tangent, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Binormal, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.Bones, numVertex, pOffsetTable, iVar4, iVar5);
    WriteAttributeOffset(patchedVA.BoneWeight, numVertex, pOffsetTable, iVar4, iVar5);
}

eVertexAttributeFormat AdjustVertexAttributeFormat(eVertexAttributeFormat param_1)
{
    // FUN_00606190
    bool bVar1 = gpRender->isVertexAttributeFormatSupported(param_1);
    if (!bVar1) {
        if (param_1 == VAF_NORMAL_A2B10G10R10_SNORM_PACK32 || param_1 == VAF_NORMAL_R16G16B16A16_SNORM) {
            return VAF_NORMAL_R32G32B32_SFLOAT;
        }

        if (param_1 == VAF_TANGENT_A2B10G10R10_SNORM_PACK32) {
            return VAF_TANGENT_R32G32B32_SFLOAT;
        }
        param_1 = (eVertexAttributeFormat)((-(uint32_t)(param_1 != eVertexAttributeFormat::VAF_TANGENT_R16G16B16A16_SNORM) & 5) + 0x15);
    }
    return param_1;
}

uint32_t PrimtiveVertexAttributes::adjustAttributesFormat()
{
    // FUN_006062b0
#define OTDU_ADJUST_VAF( attrib )\
    if (attrib.Number != '\0') {\
        eVertexAttributeFormat eVar1 = AdjustVertexAttributeFormat(attrib.Format);\
        if (eVar1 == eVertexAttributeFormat::VAF_Count || eVar1 == eVertexAttributeFormat::VAF_Invalid) {\
            return 0;\
        }\
        bVar2 |= eVar1 != attrib.Format;\
        attrib.Format = eVar1;\
    }

    bool bVar2 = false;
    OTDU_ADJUST_VAF( Position );
    OTDU_ADJUST_VAF( Normal );
    OTDU_ADJUST_VAF( Diffuse );
    OTDU_ADJUST_VAF( Specular );
    OTDU_ADJUST_VAF( UV );
    OTDU_ADJUST_VAF( Tangent );
    OTDU_ADJUST_VAF( Binormal );
    OTDU_ADJUST_VAF( Bones );
    OTDU_ADJUST_VAF( BoneWeight );
    OTDU_ADJUST_VAF( Unknown1 );
    OTDU_ADJUST_VAF( Unknown2 );
    OTDU_ADJUST_VAF( Unknown3 );
    OTDU_ADJUST_VAF( Unknown4 );   

    return bVar2 + 1;
#undef OTDU_ADJUST_VAF
}

uint32_t PrimtiveVertexAttributes::calculateVertexBufferSize(uint32_t param_2)
{
    // FUN_005fe170
#define OTDU_CALC_VERTEX_ATTRIB_SIZE( attrib )\
    if (attrib.Number != 0) {\
        uVar3 += attrib.Number * GetVertexAttributeSize(attrib.Format) * param_2;\
    }

    uint32_t uVar3 = 0;
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Position );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Normal );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Diffuse );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Specular );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( UV );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Tangent );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Binormal );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( Bones );
    OTDU_CALC_VERTEX_ATTRIB_SIZE( BoneWeight );

    return uVar3;
#undef OTDU_CALC_VERTEX_ATTRIB_SIZE
}
