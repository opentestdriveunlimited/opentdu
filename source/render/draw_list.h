#pragma once

#include "render_file.h"
#include "gs_render_helper.h"
#include "core/color.h"
#include "render/3dg.h"
#include "render/setup_node.h"

#include <Eigen/Dense>

struct Material;
struct GPUBuffer;

struct DrawStreams {
    uint8_t Normals;
    uint8_t Diffuse;
    uint8_t Specular;
    uint8_t UVMap;
    uint8_t Tangent;
    uint8_t Binormal;
    uint8_t Index;
    uint8_t Custom;

    DrawStreams();
    DrawStreams& operator = (DrawStreams& other);
};

static constexpr uint32_t kGeometryBufferMagic  = 0x54534c44; // DLST (DrawLiST)

// This is only used to match what TDU1 uses (TDU1 uses C arrays)
static constexpr uint32_t kDrawCommandsInitialNumVertexAttributes = 16; 

struct GeometryBuffer {
    RenderFile::Section Section;

    GPUBuffer* pGPUBuffer;
    void* pCPUBuffer;
    uint32_t ByteSize;
    int32_t LockStart;
    int32_t LockEnd;
    void* pCPUBufferCopy;
    uint16_t DataOffset;
    uint8_t bLocked;
    uint8_t Flags;
    GPUBuffer* pGPUBufferCopy;
    char pUnknown[16];

    void* lock(const uint32_t startOffset, const uint32_t length);
};

struct DrawPrimitive {
    RenderFile::Section Section;
    Primitive           Primitive;
    uint32_t            pOffsets[32];
};

struct ActivePrimitive {
    Primitive* pPrimitive;
    uint32_t* pOffsets;
    uint32_t* pVertexIndices;
    Eigen::Vector3f* pVertexPosition;
    Eigen::Vector3f* pVertexNormal[kDrawCommandsInitialNumVertexAttributes];
    uint32_t* pVertexDiffuse[kDrawCommandsInitialNumVertexAttributes];
    uint32_t* pVertexSpecular[kDrawCommandsInitialNumVertexAttributes];
    Eigen::Vector2f* pVertexUV[kDrawCommandsInitialNumVertexAttributes];
    Eigen::Vector3f* pVertexTangent[kDrawCommandsInitialNumVertexAttributes];
    Eigen::Vector3f* pVertexBinormal[kDrawCommandsInitialNumVertexAttributes];

    ActivePrimitive();
};

class DrawList {
public:
    DrawList();
    ~DrawList();

    bool initialize(
        uint32_t numPrimitives,
        uint32_t numVertices,
        uint32_t numIndices,
        bool param_7,
        DrawStreams* param_8,
        char* pName,
        bool bIndexed
    );

    void destroy();
    void reset();

    bool beginPrimitive(ePrimitiveType param_2, int32_t numVertex);
    void pushVertex(float x, float y, float z);
    int32_t commitPrimitive();

    void setDiffuse( const uint32_t index, const ColorRGBA& color );
    void setTexCoords( const uint32_t index, const Eigen::Vector2f& coords );
    void setActiveMaterial( Material* pMaterial );
    void setActiveTransformMatrix( Eigen::Matrix4f& matrix );
    void setIdentityTransform();

    uint32_t getNumPrimitives() const;
    Material* getMaterialAtIndex( const uint32_t index ) const { return ppMaterials[index]; }
    SetupGraph* getSetupAtIndex( const uint32_t index ) const { return ppSetups[index]; }
    const Eigen::Matrix4f& getMatrixAtIndex( const uint32_t index ) const { return pMatrices[index]; }
    const DrawPrimitive& getPrimitiveAtIndex( const uint32_t index ) const { return pMemPrimsList[index]; }
    
private:
    struct Pool {
        Pool();
        ~Pool();

        void allocate( uint32_t numPrimitive, 
            uint32_t numVertices, 
            uint32_t numIndices, 
            const DrawStreams* param_4 = nullptr,
            const bool param_5 = false );

        void* pBase;
        uint8_t* pCurrent;
        uint32_t maxSize;
        uint32_t currentUsage;
    };

    struct ActiveDrawCommand {
        Eigen::Matrix4f                 TransformMat;
        Material*                       pMaterial;
        SetupGraph                      Setup;
        std::vector<Eigen::Vector3f>    Normals;
        std::vector<Eigen::Vector3f>    Tangents;
        std::vector<Eigen::Vector3f>    Binormals;
        std::vector<Eigen::Vector2f>    UVMap;
        std::vector<uint32_t>           VertexColorDiffuse;
        std::vector<uint32_t>           VertexColorSpecular;

        ActiveDrawCommand();
    };

private:
    std::string         name;
    DrawStreams         streams;
    Pool                memoryPool;
    ActiveDrawCommand   activeCommand;
    ActivePrimitive     activePrimitive;

    GeometryBuffer*     pVertexBuffer;
    GeometryBuffer*     pIndexBuffer;
    DrawPrimitive*      pMemPrimsList;
    Eigen::Matrix4f*    pMatrices;
    Material**          ppMaterials;
    SetupGraph**        ppSetups;

    DrawList*           pCopy;
    uint32_t            flags;
    
    uint32_t            numPrimitives;
    uint32_t            numVertices;
    uint32_t            numIndex;
    
    uint32_t            maxIndex;
    uint32_t            maxBreak;
    
    uint32_t            vertexSize;
    uint32_t            indexSize;
    uint32_t            currentVertexOffset;

    uint32_t            primMaxVertex;
    uint32_t            primMaxIndex;

    uint32_t            pNormalsOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];
    uint32_t            pDiffuseOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];
    uint32_t            pSpecularOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];
    uint32_t            pUVOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];
    uint32_t            pTangentOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];
    uint32_t            pBinormalOffsetIndexes[kDrawCommandsInitialNumVertexAttributes];

    uint8_t             bInitialized : 1;
    uint8_t             bDynamic : 1;

private:
    bool allocateBuffers(bool bImmediateUpload);
    void updateGPUBuffer( GeometryBuffer* pGeometryBuffer );
    void initializePrimitiveList();

    PrimtiveVertexAttributes generateListVertexAttributes() const;
    
    bool initializePrimitive(ePrimitiveType param_2, uint32_t numVertex, uint32_t numIndices);
};
