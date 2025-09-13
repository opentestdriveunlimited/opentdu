#pragma once

#include "render_file.h"
#include "gs_render_helper.h"

struct LOD;

static constexpr uint32_t kPrimitiveMagic       = 0x4d495250; // PRIM (PRIMitive)

struct PrimtiveVertexAttributes {
    static constexpr uint32_t kMaxNumAttributes = 14;

    struct Stream {
        uint8_t Number;
        eVertexAttributeFormat Format;
        uint8_t Unknown;
        uint8_t Offset;
    };
    
    Stream Position;
    Stream Normal;
    Stream Diffuse;
    Stream Specular;
    Stream UV;
    Stream Tangent;
    Stream Binormal;
    Stream Bones;
    Stream BoneWeight;
    Stream Unknown1;
    Stream Unknown2;
    Stream Unknown3;
    Stream Unknown4;

    uint8_t NumAttributes;
    uint8_t __PADDING__[3]; // This is either padding or unknown bytes

    inline uint32_t getNumAttributes() const
    {
        return Unknown4.Number + Unknown3.Number + Unknown2.Number + Unknown1.Number + BoneWeight.Number
            + Bones.Number + Binormal.Number + Tangent.Number + UV.Number + Specular.Number + Diffuse.Number
            + Normal.Number + Position.Number;
    }

    uint32_t adjustAttributesFormat();
    uint32_t calculateVertexBufferSize(uint32_t param_2);
};
OTDU_SIZE_MUST_MATCH(PrimtiveVertexAttributes, 0x38);

struct Primitive {
    uint32_t                    Flags;
    ePrimitiveType              Type;
    x86Pointer_t                pIndexBuffer;
    x86Pointer_t                pVertexBuffer;
    x86Pointer_t                pBonesBuffer;
    x86Pointer_t                pVertexDeclaration;
    uint8_t                     bUploaded;
    uint8_t                     NumBones;
    uint16_t                    MaterialID;
    uint16_t                    IndexType;
    uint8_t                     __PADDING__[2]; // Explicit padding to match 3DG declaration. DO NOT REMOVE
    uint32_t                    VertexOffset;
    uint32_t                    NumVertex;
    uint32_t                    IndexOffset;
    uint32_t                    NumIndex;
    PrimtiveVertexAttributes    VertexStreams;
    x86Pointer_t                pOTNode;
    x86Pointer_t                pOTNodeMaterial;
    x86Pointer_t                pMorphTarget;
    uint32_t                    __PADDING2__;
    x86Pointer_t                pVertexAttributeOffsets;
};
OTDU_SIZE_MUST_MATCH(Primitive, 0x7c);

class Render3DG : public RenderFile {
public:
    Render3DG();
    ~Render3DG();

    bool initialize(RenderFile::Section* pSection);
    virtual bool parseSection(RenderFile::Section* pSection) override;
    virtual void unparseSection(RenderFile::Section* pSection) override;

    static bool UploadPrimitiveToGPU(Primitive* param_1, LOD* param_2);
    static void CreateVertexDeclaration(Primitive* param_1);

private:
    RenderFile::Section* pGeometryArray;

private:
    bool parsePrimitiveSection(RenderFile::Section* pSection);
    void unstreamPrimitive(RenderFile::Section* pSection);
    
    void unstreamHeightmap(Primitive* param_1);
    void unstreamGeometry(Primitive* param_1);

    static bool WritePrimitiveHeader(int8_t *pPatchedHeader, PrimtiveVertexAttributes &patchedVA, int8_t *pOriginalHeader, const PrimtiveVertexAttributes &originalVA, uint32_t numVertex);
    static bool WritePatchedVertices(int8_t *pPatchedHeader, PrimtiveVertexAttributes::Stream& pPatched, int8_t *pOriginalHeader, const PrimtiveVertexAttributes::Stream& pOriginal, uint32_t numVertex);
    static void WriteAttributeOffsets(int8_t *pOffsetTable, PrimtiveVertexAttributes &patchedVA, uint32_t numVertex);
};
