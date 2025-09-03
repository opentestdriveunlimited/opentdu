#pragma once

#include "render_file.h"
#include "gs_render_helper.h"

static constexpr uint32_t kPrimitiveMagic       = 0x4d495250; // PRIM (PRIMitive)

struct PrimtiveVertexAttributes {
    static constexpr uint32_t kMaxNumAttributes = 14;

    struct Stream {
        uint8_t Number;
        uint8_t Format;
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
};

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
};
OTDU_SIZE_MUST_MATCH(Primitive, 0x74);

class Render3DG : public RenderFile {
public:
    Render3DG();
    ~Render3DG();

    bool initialize(RenderFile::Section* pSection);
    virtual bool parseSection(RenderFile::Section* pSection) override;
    virtual void unparseSection(RenderFile::Section* pSection) override;

private:
    RenderFile::Section* pGeometryArray;

private:
    bool parsePrimitiveSection(RenderFile::Section* pSection);
    void unstreamPrimitive(RenderFile::Section* pSection);
    
    void unstreamHeightmap(Primitive* param_1);
    void unstreamGeometry(Primitive* param_1);
};
