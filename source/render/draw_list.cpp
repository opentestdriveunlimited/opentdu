#include "shared.h"
#include "draw_list.h"
#include "geometry_buffer.h"
#include "gs_render.h"

RenderPool<DrawList> gDrawListPool; // DAT_00fad4d8

static uint32_t CalculatePoolSize( 
    uint32_t numPrimitive, 
    uint32_t numVertices, 
    uint32_t numIndices, 
    const DrawStreams* param_4, 
    const bool param_5 )
{
    uint32_t uVar1;
    int iVar2;

    if ((numIndices & 1) != 0) {
        numIndices = numIndices + 1;
    }
    if ((numVertices & 1) != 0) {
        numVertices = numVertices + 1;
    }
    iVar2 = numPrimitive + 1;
    if (!param_5) {
        numIndices = numVertices + numIndices * 2;
    }
    uVar1 = iVar2 * 0x108 + 0x10;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    uVar1 = uVar1 + iVar2 * 0x40;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    uVar1 = uVar1 + iVar2 * 4;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    uVar1 = uVar1 + iVar2 * 4;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    if (param_4 == nullptr) {
        iVar2 = 0x24;
    } else {
        iVar2 = (((uint32_t)param_4->Binormal + (uint32_t)param_4->Tangent + 1 + (uint32_t)param_4->Normals) * 3 +
                (uint32_t)param_4->UVMap * 2 + (uint32_t)param_4->Specular + (uint32_t)param_4->Diffuse) * 4;
    }
    uVar1 = uVar1 + 0x70 + iVar2 * numVertices;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    if ((param_4 == nullptr) || (iVar2 = 2, param_4->Index == 0)) {
        iVar2 = 4;
    }
    uVar1 = uVar1 + 0x70 + iVar2 * numIndices;
    if ((uVar1 & 0xf) != 0) {
        uVar1 = ((uVar1 >> 4) + 1) * 0x10;
    }
    return uVar1 + 0xfff & 0xfffff000;
}

DrawStreams::DrawStreams()
    : Normals( 1 )
    , Diffuse( 1 )
    , Specular( 0 )
    , UVMap( 1 )
    , Tangent( 0 )
    , Binormal( 0 )
    , Index( 0 )
    , Custom( 0 )
{

}

DrawStreams& DrawStreams::operator= (DrawStreams& other)
{
    Normals = other.Normals;
    Diffuse = other.Diffuse;
    Specular = other.Specular;
    UVMap = other.UVMap;
    Tangent = other.Tangent;
    Binormal = other.Binormal;
    Index = other.Index;
    Custom = other.Custom;
    return *this;
}

DrawList::Pool::Pool()
    : pBase( nullptr )
    , pCurrent( nullptr )
    , maxSize( 0u )
    , currentUsage( 0u )
{
}

DrawList::Pool::~Pool()
{
    if (pBase != nullptr) {
        TestDrive::FreeAligned( pBase );
        pBase = nullptr;
    }
}

void DrawList::Pool::allocate(
    uint32_t numPrimitive, 
    uint32_t numVertices, 
    uint32_t numIndices, 
    const DrawStreams *param_4, 
    const bool param_5
)
{
    uint32_t poolSize = CalculatePoolSize(numPrimitive, numVertices, numIndices, param_4, param_5);

    void* pMemory = TestDrive::Alloc(poolSize);
    OTDU_ASSERT(pMemory);
    
    pBase = pMemory;
    pCurrent = reinterpret_cast<uint8_t*>( pMemory );
     // TODO: Check why they reserve 16 bytes? (header? tmp memory?)
    maxSize = poolSize - 0x10;
    currentUsage = 0;
}

static constexpr uint32_t kDefaultVertexStride = 0x24;
static constexpr uint32_t kDefaultIndexSize = sizeof(uint16_t);

DrawList::DrawList()
    : name("no name")
    , streams()
    , memoryPool()
    , activeCommand()
    , activePrimitive()
    , pVertexBuffer( nullptr )
    , pIndexBuffer( nullptr )
    , pCopy( nullptr )
    , flags( 0u )
    , numPrimitives( 0u )
    , numVertices( 0u )
    , numIndex( 0u )
    , maxIndex( 0u )
    , maxBreak( 0u )
    , vertexSize( kDefaultVertexStride )
    , indexSize( kDefaultIndexSize )
    , currentVertexOffset( 0u )
    , primMaxVertex( 0u )
    , primMaxIndex( 0u )
    , bInitialized( false )
    , bDynamic( false )
{
    memset(pNormalsOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
    memset(pDiffuseOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
    memset(pSpecularOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
    memset(pUVOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
    memset(pTangentOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
    memset(pBinormalOffsetIndexes, 0, sizeof(uint32_t) * kDrawCommandsInitialNumVertexAttributes);
}

DrawList::~DrawList()
{
    
}

bool DrawList::initialize(
    uint32_t newNumPrimitives, 
    uint32_t newNumVertices, 
    uint32_t numIndices,
    bool param_7, 
    DrawStreams* param_8, 
    char* pName, 
    bool param_10 )
{
    if (pName != nullptr) {
        name = std::string(pName);
    }

    if ((numIndices & 1) != 0) {
        numIndices = numIndices + 1;
    }

    if ((newNumVertices & 1) != 0) {
        newNumVertices = newNumVertices + 1;
    }

    numPrimitives = newNumPrimitives;
    numVertices = newNumVertices;
    bDynamic = param_7;

    if (param_10) {
        maxIndex = numIndices;
        if (numIndices <= numVertices) {
            maxBreak = 0;
        } else { 
            numIndices = (numIndices - numVertices) >> 1;
            maxBreak = numIndices;
        }
    } else {
      maxIndex = numVertices + numIndices * 2;
      maxBreak = numIndices;
    }
    
    if (param_8 != nullptr) {
        streams = *param_8;
        vertexSize = ((streams.Binormal + streams.Tangent + 1 +
                    streams.Normals) * 3 + streams.UVMap * 2 +
                    streams.Specular + streams.Diffuse) * 4;
    }
    
    if ( streams.Index == 0 ) {
        indexSize = 4;
    }

    memoryPool.allocate( numPrimitives, numVertices, numIndices, param_8, param_10 );

    pVertexBuffer = reinterpret_cast<GeomtryBufferWithHeader*>( memoryPool.pCurrent );

    uint32_t vertexMemoryUsage = vertexSize * numVertices + 0x7f & 0xfffffff0;
    memoryPool.currentUsage += vertexMemoryUsage;
    memoryPool.pCurrent += vertexMemoryUsage;
    
    memset(pVertexBuffer, 0, sizeof(GeomtryBufferWithHeader));
    pVertexBuffer->Section.Type = kGeometryBufferMagic;

    pIndexBuffer = reinterpret_cast<GeomtryBufferWithHeader*>( memoryPool.pCurrent );

    uint32_t indicesMemoryUsage = maxIndex * indexSize + 0x7f & 0xfffffff0;
    memoryPool.currentUsage += indicesMemoryUsage;
    memoryPool.pCurrent += indicesMemoryUsage;

    memset(pIndexBuffer, 0, sizeof(GeometryBuffer));
    pIndexBuffer->Section.Type = kGeometryBufferMagic;

    bool bVar8 = allocateBuffers( false );

    if (!bVar8) {
      return false;
    }
    
    pMemPrimsList = reinterpret_cast<DrawPrimitive*>( memoryPool.pCurrent );

    uint32_t primitiveMemoryUsage = numPrimitives * sizeof( DrawPrimitive ) + 0x7f & 0xfffffff0;
    memoryPool.currentUsage += primitiveMemoryUsage;
    memoryPool.pCurrent += primitiveMemoryUsage;

    pMatrices = reinterpret_cast<Eigen::Matrix4f*>( memoryPool.pCurrent );

    uint32_t matricesMemoryUsage = numPrimitives * sizeof(Eigen::Matrix4f);
    memoryPool.currentUsage += matricesMemoryUsage;
    memoryPool.pCurrent += matricesMemoryUsage;

    ppMaterials = reinterpret_cast<Material**>( memoryPool.pCurrent );

    uint32_t materialsMemoryUsage = numPrimitives * sizeof(Material*) + 0xf & 0xfffffff0;
    memoryPool.currentUsage += materialsMemoryUsage;
    memoryPool.pCurrent += materialsMemoryUsage;

    ppSetups = reinterpret_cast<SetupGraph**>( memoryPool.pCurrent );

    uint32_t flagsMemoryUsage = numPrimitives * sizeof(SetupGraph*) + 0xf & 0xfffffff0;
    memoryPool.currentUsage += flagsMemoryUsage;
    memoryPool.pCurrent += flagsMemoryUsage;

    initializePrimitiveList();
    pCopy = this;

    // FUN_00512310(this); // TODO: register this drawlist in some global class. Do we need this?

    return true;
}

void DrawList::destroy()
{
    OTDU_UNIMPLEMENTED;
    // gpRender->getRenderDevice()->destroyBuffer( pIndexBuffer->pGPUBuffer );
    // gpRender->getRenderDevice()->destroyBuffer( pVertexBuffer->pGPUBuffer );
}

void DrawList::reset()
{
    numPrimitives = 0;
    numVertices = 0;
    numIndex = 0;
    currentVertexOffset = 0;
}

bool DrawList::beginPrimitive(ePrimitiveType param_2, int32_t numVertex)
{
    bool bVar1;
    uint32_t numIndicesToAdd;
    
    numIndicesToAdd = numVertex + maxBreak * 2;
    if (maxIndex < numIndex + numIndicesToAdd) {
      numIndicesToAdd = maxIndex - numIndex;
    }
    bVar1 = initializePrimitive(param_2,numVertex,numIndicesToAdd);
    if (bVar1) {
      flags |= 2;
      return true;
    }
    return false;
}

void DrawList::pushVertex(float x, float y, float z)
{
    *activePrimitive.pVertexPosition = Eigen::Vector3f(x, y, z);
    activePrimitive.pVertexPosition++;
    OTDU_UNIMPLEMENTED; // TODO:
}

int32_t DrawList::commitPrimitive()
{
    numVertices += activePrimitive.pPrimitive->NumVertex;
    numIndex += activePrimitive.pPrimitive->NumIndex;
    numPrimitives++;
    flags &= 0xfffffffc;

    return numPrimitives - 1;
}

void DrawList::setDiffuse(const uint32_t index, const ColorRGBA &color)
{
    activeCommand.VertexColorDiffuse[index] = color.Color;
}

void DrawList::setTexCoords(const uint32_t index, const Eigen::Vector2f &coords)
{
    activeCommand.UVMap[index] = coords;
}

void DrawList::setActiveMaterial(Material *pMaterial)
{
    activeCommand.pMaterial = pMaterial;
}

void DrawList::setActiveTransformMatrix(Eigen::Matrix4f &matrix)
{
    activeCommand.TransformMat = matrix;
}

void DrawList::setIdentityTransform()
{
    activeCommand.TransformMat = Eigen::Matrix4f::Identity();
}

uint32_t DrawList::getNumPrimitives() const
{
    return numPrimitives;
}

void DrawList::Create(DrawList *param_1)
{
    // FUN_00512340
    if (param_1 != nullptr) {
        param_1->allocateBuffers(true);
    }
}

void DrawList::initializePrimitiveList()
{
    PrimtiveVertexAttributes listVertexAttributes = generateListVertexAttributes();

    for (uint32_t i = 0; i < numPrimitives; i++) {
        DrawPrimitive& drawPrimitive = pMemPrimsList[i];
        memset(&drawPrimitive, 0, sizeof(DrawPrimitive));

        drawPrimitive.Section.Type = kPrimitiveMagic;
        drawPrimitive.Section.VersionMajor = 0;
        drawPrimitive.Section.VersionMinor = 0;

        drawPrimitive.Primitive.pIndexBuffer = &pIndexBuffer->Section;
        drawPrimitive.Primitive.pVertexBuffer = &pVertexBuffer->Section;
        drawPrimitive.Primitive.bUploaded = true;   
        drawPrimitive.Primitive.MaterialID = streams.Index != '\0';
        drawPrimitive.Primitive.VertexStreams = listVertexAttributes;
    }

    // Update stream offsets.
    uint32_t uVar6 = 1;
    for (uint32_t i = 0; i < streams.Normals; i++) {
      pNormalsOffsetIndexes[i] = uVar6;
      uVar6++;
    }

    for (uint32_t i = 0; i < streams.Diffuse; i++) {
      pDiffuseOffsetIndexes[i] = uVar6;
      uVar6++;
    }

    for (uint32_t i = 0; i < streams.Specular; i++) {
      pSpecularOffsetIndexes[i] = uVar6;
      uVar6++;
    }

    for (uint32_t i = 0; i < streams.UVMap; i++) {
      pUVOffsetIndexes[i] = uVar6;
      uVar6++;
    }

    for (uint32_t i = 0; i < streams.Tangent; i++) {
      pTangentOffsetIndexes[i] = uVar6;
      uVar6++;
    }

    for (uint32_t i = 0; i < streams.Binormal; i++) {
      pBinormalOffsetIndexes[i] = uVar6;
      uVar6++;
    }
}

PrimtiveVertexAttributes DrawList::generateListVertexAttributes() const
{
    PrimtiveVertexAttributes local_38;
    memset(&local_38, 0, sizeof(PrimtiveVertexAttributes));
    local_38.Position.Number = '\x01';
    local_38.Position.Format = eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT;
    local_38.Position.Unknown = '\0';
    local_38.Position.Offset = GetVertexAttributeSize(eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT);
    local_38.Normal.Number = streams.Normals;
    local_38.Normal.Format = eVertexAttributeFormat::VAF_NORMAL_R32G32B32_SFLOAT;
    local_38.Normal.Unknown = '\0';
    uint32_t uVar1 = GetVertexAttributeSize(eVertexAttributeFormat::VAF_NORMAL_R32G32B32_SFLOAT);
    local_38.Diffuse.Number = streams.Diffuse;
    local_38.Diffuse.Format = eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM;
    local_38.Diffuse.Unknown = '\0';
    local_38.Normal.Offset = uVar1;
    local_38.Diffuse.Offset = GetVertexAttributeSize(eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM);
    local_38.Specular.Number = streams.Specular;
    local_38.UV.Number = streams.UVMap;
    local_38.Specular.Format = eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM;
    local_38.Specular.Unknown = '\0';
    local_38.UV.Format = eVertexAttributeFormat::VAF_TEXCOORD_R32G32_SFLOAT;
    local_38.UV.Unknown = '\0';
    local_38.Specular.Offset = local_38.Diffuse.Offset;
    local_38.UV.Offset = GetVertexAttributeSize(eVertexAttributeFormat::VAF_TEXCOORD_R32G32_SFLOAT);
    local_38.Tangent.Number = streams.Tangent;
    local_38.Tangent.Format = eVertexAttributeFormat::VAF_TANGENT_R32G32B32_SFLOAT;
    local_38.Tangent.Unknown = '\0';
    local_38.Tangent.Offset = GetVertexAttributeSize(eVertexAttributeFormat::VAF_TANGENT_R32G32B32_SFLOAT);
    local_38.Binormal.Number = streams.Binormal;
    local_38.Binormal.Offset = uVar1;
    uint32_t uVar6 = 0;
    local_38.Binormal.Format = eVertexAttributeFormat::VAF_NORMAL_R32G32B32_SFLOAT;
    local_38.Binormal.Unknown = '\0';
    local_38.NumAttributes =
         local_38.Unknown4.Number + local_38.Unknown3.Number + local_38.Unknown2.Number +
         local_38.Unknown1.Number + local_38.BoneWeight.Number + local_38.Bones.Number +
         local_38.Binormal.Number + local_38.Tangent.Number + local_38.UV.Number +
         local_38.Specular.Number + local_38.Diffuse.Number + local_38.Normal.Number + '\x01';
    return local_38;
}

bool DrawList::initializePrimitive(ePrimitiveType param_2, uint32_t numVertex, uint32_t numIndices)
{
    if (numVertices < numVertex + numVertex) {
      return false;
    }
    if ((numIndex + numIndices <= maxIndex) &&
       (pCopy == this)) {
      ppMaterials[numPrimitives] = activeCommand.pMaterial;
      pMatrices[numPrimitives] = activeCommand.TransformMat;
      ppSetups[numPrimitives] = &activeCommand.Setup;
      
      activePrimitive.pPrimitive = &pMemPrimsList[numPrimitives].Primitive;
      
      primMaxVertex = numVertex;
      primMaxIndex = numIndices;
      activePrimitive.pPrimitive->Type = param_2;
      activePrimitive.pPrimitive->NumVertex = 0;
      activePrimitive.pPrimitive->NumIndex = 0;
      activePrimitive.pPrimitive->IndexOffset = numIndex;

      uint32_t* puVar9 = pMemPrimsList[numPrimitives].pOffsets;
      activePrimitive.pOffsets = puVar9;
      
      int32_t iVar5 = numVertex * sizeof(Eigen::Vector3f);
      activePrimitive.pVertexPosition = static_cast<Eigen::Vector3f*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar5));
      *puVar9 = currentVertexOffset;
      currentVertexOffset += iVar5;
      puVar9++;

      // TODO: Quantify
      for (uint32_t i = 0; i < streams.Normals; i++) {
        activePrimitive.pVertexNormal[i] = static_cast<Eigen::Vector3f*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar5));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar5;
        puVar9++;
      }
      
      int32_t iVar4 = numVertex * sizeof(uint32_t);
      
      for (uint32_t i = 0; i < streams.Diffuse; i++) {
        activePrimitive.pVertexDiffuse[i] = static_cast<uint32_t*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar4));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar4;
        puVar9++;
      }
    
      for (uint32_t i = 0; i < streams.Specular; i++) {
        activePrimitive.pVertexSpecular[i] = static_cast<uint32_t*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar4));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar4;
        puVar9++;
      }
     
      iVar4 = numVertex * sizeof(Eigen::Vector2f);
      
      for (uint32_t i = 0; i < streams.UVMap; i++) {
        activePrimitive.pVertexUV[i] = static_cast<Eigen::Vector2f*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar4));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar4;
        puVar9++;
      }

      for (uint32_t i = 0; i < streams.Tangent; i++) {
        activePrimitive.pVertexTangent[i] = static_cast<Eigen::Vector3f*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar5));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar5;
        puVar9++;
      }

      for (uint32_t i = 0; i < streams.Binormal; i++) {
        activePrimitive.pVertexBinormal[i] = static_cast<Eigen::Vector3f*>(pVertexBuffer->Buffer.lock(currentVertexOffset, iVar5));
        *puVar9 = currentVertexOffset;
        currentVertexOffset += iVar5;
        puVar9++;
      }

      uint32_t uVar7 = indexSize;
      uint32_t indiceLock = activePrimitive.pPrimitive->IndexOffset * uVar7;
      uint32_t indiceLockSize = numIndices * uVar7;

      activePrimitive.pVertexIndices = static_cast<uint32_t*>(pVertexBuffer->Buffer.lock(indiceLock, indiceLockSize));
      *puVar9 = currentVertexOffset;
      return true;
    }
    return false;
}

bool DrawList::allocateBuffers(bool bImmediateUpload)
{
    // FUN_00507950
    uint32_t bufferSize = numVertices * vertexSize;

    GPUBufferDesc vertexBufferDesc;
    vertexBufferDesc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
    vertexBufferDesc.Size = bufferSize;
    vertexBufferDesc.Stride = vertexSize;
    vertexBufferDesc.bDynamic = bDynamic;

    pVertexBuffer->Buffer.pGPUBuffer = gpRender->getRenderDevice()->createBuffer( &vertexBufferDesc );
    OTDU_ASSERT( pVertexBuffer->Buffer.pGPUBuffer );

    if (pVertexBuffer->Buffer.pGPUBuffer == nullptr) {
        return false;
    }

    pVertexBuffer->Buffer.pCPUBuffer = pVertexBuffer + 1;
    pVertexBuffer->Buffer.ByteSize = bufferSize;
    pVertexBuffer->Buffer.bLocked = false;
    pVertexBuffer->Buffer.pCPUBufferCopy = nullptr;
    pVertexBuffer->Buffer.pGPUBufferCopy = nullptr;
    pVertexBuffer->Buffer.Flags = 0;
    pVertexBuffer->Buffer.LockStart = -1;
    pVertexBuffer->Buffer.LockEnd = 0;
    if (bDynamic) {
        pVertexBuffer->Buffer.Flags = ( pVertexBuffer->Buffer.Flags & 0xf1 | 1 );
    }
    
    uint32_t indexBufferSize = maxIndex * indexSize;

    GPUBufferDesc indexBufferDesc;
    indexBufferDesc.BufferType = GPUBufferDesc::Type::BT_IndexBuffer;
    indexBufferDesc.Size = indexBufferSize;
    indexBufferDesc.Stride = (streams.Index != '\0') ? sizeof(uint16_t) : sizeof(uint32_t);
    indexBufferDesc.bDynamic = bDynamic;

    pIndexBuffer->Buffer.pGPUBuffer = gpRender->getRenderDevice()->createBuffer( &indexBufferDesc );
    OTDU_ASSERT( pIndexBuffer->Buffer.pGPUBuffer );

    if ( pIndexBuffer->Buffer.pGPUBuffer != nullptr ) {
        return false;
    }
    
    pIndexBuffer->Buffer.pCPUBuffer = pIndexBuffer + 1;
    pIndexBuffer->Buffer.ByteSize = indexBufferSize;
    pIndexBuffer->Buffer.bLocked = false;
    pIndexBuffer->Buffer.pCPUBufferCopy = nullptr;
    pIndexBuffer->Buffer.pGPUBufferCopy = nullptr;
    pIndexBuffer->Buffer.Flags = 0x20;
    pIndexBuffer->Buffer.LockStart = -1;
    pIndexBuffer->Buffer.LockEnd = 0;
    if (bDynamic) {
        pIndexBuffer->Buffer.Flags = ( pVertexBuffer->Buffer.Flags & 0xf1 | 1 );
    }

    if (bImmediateUpload) {
        pVertexBuffer->Buffer.LockStart = 0;
        pVertexBuffer->Buffer.LockEnd = pVertexBuffer->Buffer.ByteSize;
        pVertexBuffer->Buffer.bLocked = true;

        pIndexBuffer->Buffer.LockStart = 0;
        pIndexBuffer->Buffer.LockEnd = pIndexBuffer->Buffer.ByteSize;
        pIndexBuffer->Buffer.bLocked = true;
    
        pVertexBuffer->Buffer.uploadVerticesToGPU();
        pIndexBuffer->Buffer.uploadIndicesToGPU();
      }
      
    return true;
}

DrawList::ActiveDrawCommand::ActiveDrawCommand()
    : TransformMat( Eigen::Matrix4f::Identity() )
    , pMaterial( nullptr )
    , Setup{}
{
    Normals.reserve( kDrawCommandsInitialNumVertexAttributes );
    Tangents.reserve( kDrawCommandsInitialNumVertexAttributes );
    Binormals.reserve( kDrawCommandsInitialNumVertexAttributes );
    UVMap.reserve( kDrawCommandsInitialNumVertexAttributes );
    VertexColorDiffuse.reserve( kDrawCommandsInitialNumVertexAttributes );
    VertexColorSpecular.reserve( kDrawCommandsInitialNumVertexAttributes );
}

ActivePrimitive::ActivePrimitive()
    : pPrimitive( nullptr )
    , pOffsets( nullptr )
    , pVertexIndices( nullptr )
    , pVertexPosition( nullptr )
{
    memset( pVertexNormal, 0, sizeof( Eigen::Vector3f* ) * 16 );
    memset( pVertexDiffuse, 0, sizeof( uint32_t* ) * 16 );
    memset( pVertexSpecular, 0, sizeof( uint32_t* ) * 16 );
    memset( pVertexUV, 0, sizeof( Eigen::Vector2f* ) * 16 );
    memset( pVertexTangent, 0, sizeof( Eigen::Vector3f* ) * 16 );
    memset( pVertexBinormal, 0, sizeof( Eigen::Vector3f* ) * 16 );
}
