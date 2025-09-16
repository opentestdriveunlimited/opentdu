#include "shared.h"
#include "instance.h"

#include "render/camera/camera_base.h"
#include "render/gs_render.h"
#include "render/render_buckets.h"
#include "render/3dg.h"

Instance::Instance()
    : hashcode( 0ull )
    , flags( 0 )
    , pBankMat( nullptr )
    , matrix( Eigen::Matrix4f::Identity() )
    , biggestScale( 0.0f )
    , determinant( 0.0f )
    , distanceToCurrentLOD( 0.0f )
    , numLODs( 0 )
    , currentLODIndex( 0 )
    , pBoneArray( nullptr )
    , color()
    , uvTime( 0.0f )
    , uvaFlags( 0 )
    , setupGraph{}
    , pInstanceDef( nullptr )
{
    
}

Instance::~Instance()
{
}

void Instance::calculateLOD(Camera* param_2)
{
    // FUN_0050df70
    float fVar4 = 0.0f;
    if ((numLODs == 0) ||
        ((numLODs == 1 &&
        (fVar4 = lods[0].Distances[0], isnan(fVar4) != (fVar4 == 0.0f))))) {
        currentLODIndex = 0;
        distanceToCurrentLOD = 0.0f;
        return;
    }
    Eigen::Vector4f local_30 = matrix.row(3);
    Eigen::Vector4f local_20 = param_2->getWorldToCam() * local_30;
    distanceToCurrentLOD = local_20.norm();

    float fVar5 = param_2->getTanHalfFov();
    float fVar6 = gpRender->getLODFactor();
    uint16_t uVar2 = 0;
    fVar4 = (fVar5 / (biggestScale * biggestScale)) * fVar5 * fVar4 * fVar6 * fVar6;
    distanceToCurrentLOD = fVar4;

    if (numLODs != 0) {
        for (; uVar2 < numLODs; uVar2++) {
            if (fVar4 < lods[0].Distances[uVar2]) break;
        }
    }
    if (uVar2 == numLODs) {
        currentLODIndex = 4;
        return;
    }
    currentLODIndex = uVar2;
}

static eVertexAttributeFormat GetFallbackFormatForAttribute(eVertexAttributeFormat eVar1)
{
    // FUN_00606120
    if (eVar1 == eVertexAttributeFormat::VAF_Invalid || eVar1 == eVertexAttributeFormat::VAF_Count) {
        return eVar1;
    }
    if (eVar1 != eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32 && eVar1 != eVertexAttributeFormat::VAF_NORMAL_R16G16B16A16_SNORM) {
        if (eVar1 == eVertexAttributeFormat::VAF_TANGENT_A2B10G10R10_SNORM_PACK32) {
            return eVertexAttributeFormat::VAF_Invalid;
        }
        return (eVertexAttributeFormat)((-(uint32_t)(eVar1 != eVertexAttributeFormat::VAF_TANGENT_R16G16B16A16_SNORM) & 5) + 0x15);
    }
    return eVertexAttributeFormat::VAF_Invalid;
}

static bool PatchVertexLayoutForDevice(VertexLayoutAttribute* param_1)
{
    // FUN_00606240
    bool bVar2 = false;
    bool cVar2 = false;
    do {
        if (param_1->Stream == 0xff) {
            return (cVar2 + 1);
        }
        bVar2 = gpRender->getRenderDevice()->isVertexAttributeFormatSupported(param_1->Type);
        if (!bVar2) {
            cVar2 = true;
            do {
                param_1->Type = GetFallbackFormatForAttribute(param_1->Type);
                if (param_1->Type == eVertexAttributeFormat::VAF_Invalid) {
                    return 0;
                }
                bVar2 = gpRender->getRenderDevice()->isVertexAttributeFormatSupported(param_1->Type);
            } while (!bVar2);
        }
        param_1++;
    } while ( true );
}

static constexpr uint32_t kMaxNumInstances = 1000;

struct InstanceEntry {
    float ModelMatrix[0xc]; // 4x3 Transform Matrix
    uint32_t InstanceColor;
    uint32_t __PADDING__[3];
};
OTDU_SIZE_MUST_MATCH(InstanceEntry, 0x40);

// DAT_00f47700
static VertexLayoutAttribute gInstancingVertexLayoutPositionNormal[] = {
    { 0, 0,     eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT,              0, 0x0, 0x0 },
    { 1, 0,     eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32,        0, 0x3, 0x0 },
    { 2, 0,     eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x0 },
    { 2, 0x10,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x1 },
    { 2, 0x20,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x2 },
    { 2, 0x30,  eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    kVertexLayoutSentinel
};

// DAT_00f47738
static VertexLayoutAttribute gInstancingVertexLayoutPositionNormalColor[] = {
    { 0, 0,     eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT,              0, 0x0, 0x0 },
    { 1, 0,     eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32,        0, 0x3, 0x0 },
    { 2, 0,     eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    { 3, 0,     eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x0 },
    { 3, 0x10,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x1 },
    { 3, 0x20,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x2 },
    { 3, 0x30,  eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    kVertexLayoutSentinel
};

// DAT_00f47778
static VertexLayoutAttribute gInstancingVertexLayoutPositionNormalUV[] = {
    { 0, 0,     eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT,              0, 0x0, 0x0 },
    { 1, 0,     eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32,        0, 0x3, 0x0 },
    { 2, 0,     eVertexAttributeFormat::VAF_TEXCOORD_R32G32_SFLOAT,                 0, 0x5, 0x0 },
    { 3, 0,     eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x0 },
    { 3, 0x10,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x1 },
    { 3, 0x20,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x2 },
    { 3, 0x30,  eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    kVertexLayoutSentinel
};

// DAT_00f477b8
static VertexLayoutAttribute gInstancingVertexLayoutPositionNormalColorUV[] = {
    { 0, 0,     eVertexAttributeFormat::VAF_POSITION_R32G32B32_SFLOAT,              0, 0x0, 0x0 },
    { 1, 0,     eVertexAttributeFormat::VAF_NORMAL_A2B10G10R10_SNORM_PACK32,        0, 0x3, 0x0 },
    { 2, 0,     eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    { 3, 0,     eVertexAttributeFormat::VAF_TEXCOORD_R32G32_SFLOAT,                 0, 0x5, 0x0 },
    { 4, 0,     eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x0 },
    { 4, 0x10,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x1 },
    { 4, 0x20,  eVertexAttributeFormat::VAF_INSTANCE_R32G32B32A32_SFLOAT,           0, 0x5, 0x2 },
    { 4, 0x30,  eVertexAttributeFormat::VAF_COLOR_B8G8R8A8_UNORM,                   0, 0xa, 0x0 },
    kVertexLayoutSentinel
};

InstanceRenderer gInstanceRenderer = {}; // DAT_00fe8a14

InstanceRenderer::InstanceRenderer()
    : pInstanceBuffer(nullptr)
    , pMappedBuffer(nullptr)
    , pVertexLayoutPositionNormal(nullptr)
    , pVertexLayoutPositionNormalColor(nullptr)
    , pVertexLayoutPositionNormalColorUV(nullptr)
    , pVertexLayoutPositionNormalUV(nullptr)
    , numInstances(0)
    , streamIndex(0)
{

}

InstanceRenderer::~InstanceRenderer()
{

}

void InstanceRenderer::destroy()
{
    // FUN_00604300
    if (pVertexLayoutPositionNormal != nullptr) {
        gpRender->getRenderDevice()->destroyVertexLayout(pVertexLayoutPositionNormal);
        pVertexLayoutPositionNormal = nullptr;
    }
    
    if (pVertexLayoutPositionNormalColor != nullptr) {
        gpRender->getRenderDevice()->destroyVertexLayout(pVertexLayoutPositionNormalColor);
        pVertexLayoutPositionNormalColor = nullptr;
    }
    
    if (pVertexLayoutPositionNormalColorUV != nullptr) {
        gpRender->getRenderDevice()->destroyVertexLayout(pVertexLayoutPositionNormalColorUV);
        pVertexLayoutPositionNormalColorUV = nullptr;
    }

    if (pVertexLayoutPositionNormalUV != nullptr) {
        gpRender->getRenderDevice()->destroyVertexLayout(pVertexLayoutPositionNormalUV);
        pVertexLayoutPositionNormalUV = nullptr;
    }

    if (pInstanceBuffer != nullptr) {
        gpRender->getRenderDevice()->destroyBuffer(pInstanceBuffer);
        pInstanceBuffer = nullptr;
    }

    numInstances = 0;
}

bool InstanceRenderer::initialize()
{
    // FUN_00604230
    PatchVertexLayoutForDevice(gInstancingVertexLayoutPositionNormal);
    PatchVertexLayoutForDevice(gInstancingVertexLayoutPositionNormalColor);
    PatchVertexLayoutForDevice(gInstancingVertexLayoutPositionNormalColorUV);
    PatchVertexLayoutForDevice(gInstancingVertexLayoutPositionNormalUV);
  
    pVertexLayoutPositionNormal = gpRender->getRenderDevice()->createVertexLayout(gInstancingVertexLayoutPositionNormal);
    if (pVertexLayoutPositionNormal == nullptr) {
        return false;
    }

    pVertexLayoutPositionNormalColor = gpRender->getRenderDevice()->createVertexLayout(gInstancingVertexLayoutPositionNormalColor);
    if (pVertexLayoutPositionNormalColor == nullptr) {
        return false;
    }
    
    pVertexLayoutPositionNormalColorUV = gpRender->getRenderDevice()->createVertexLayout(gInstancingVertexLayoutPositionNormalColorUV);
    if (pVertexLayoutPositionNormalColorUV == nullptr) {
        return false;
    }
    
    pVertexLayoutPositionNormalUV = gpRender->getRenderDevice()->createVertexLayout(gInstancingVertexLayoutPositionNormalUV);
    if (pVertexLayoutPositionNormalUV == nullptr) {
        return false;
    }

    if (pInstanceBuffer == nullptr) {
        GPUBufferDesc desc;
        desc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
        desc.bDynamic = true;
        desc.Size = 0x1900;

        pInstanceBuffer = gpRender->getRenderDevice()->createBuffer(&desc);
    }

    numInstances = 0;

    return true;
}

void InstanceRenderer::bind(Primitive *param_1)
{
    // FUN_00604380
    GPUVertexLayout* pVertexLayout = nullptr;
    if (param_1->VertexStreams.Diffuse.Number == 1) {
        if (param_1->VertexStreams.UV.Number == 1) {
            streamIndex = 4;
            pVertexLayout = pVertexLayoutPositionNormalColorUV;
        } else {
            streamIndex = 3;
            pVertexLayout = pVertexLayoutPositionNormalColor;
        }
    } else if (param_1->VertexStreams.UV.Number == 1) {
        streamIndex = 3;
        pVertexLayout = pVertexLayoutPositionNormalUV;
    } else {
        streamIndex = 2;
        pVertexLayout = pVertexLayoutPositionNormal;
    }

    gpRender->bindVertexLayout(pVertexLayout);
}

void InstanceRenderer::unbind()
{
    // FUN_006045c0
    uint32_t streamNumber = 0;
    do {
        gpRender->setStreamFrequency(streamNumber, 1);
        streamNumber++;
    } while (streamNumber <= streamIndex);

    numInstances = 0;
    pMappedBuffer = nullptr;
}

bool InstanceRenderer::uploadInstance(Instance *param_1, TransformMatrixCommand *param_2, bool bIsLastInstance)
{
    // FUN_00604400
    if (pMappedBuffer == nullptr) {
        pMappedBuffer = (InstanceEntry*)gpRender->getRenderDevice()->lockBuffer(pInstanceBuffer, 0, 0, true);
        if (pMappedBuffer == nullptr) {
            return true;
        }

        numInstances = 0;
    }

    InstanceEntry& instance = *pMappedBuffer;
    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 4; j++) {
            instance.ModelMatrix[i * 4 + j] = param_2->ModelMatrix.row(j)[i];
        }
    }
    instance.InstanceColor = ((param_1->getFlags() & 0x78) == 0) ? 0xffffffff : param_1->getColor().Color;  
    pMappedBuffer++;
    numInstances++;
    
    if (numInstances < 100 && !bIsLastInstance) {
        return true;
    }
    gpRender->getRenderDevice()->unlockBuffer(pInstanceBuffer);
    gpRender->bindVertexBuffer(pInstanceBuffer, streamIndex, 0, 0x40);

    uint32_t streamNumber_00 = 0;
    uint32_t streamNumber = streamIndex;
    if (streamIndex != 0) {
        do {
            int32_t divider = numInstances | 0x40000000;
            gpRender->setStreamFrequency(streamNumber_00, divider);
            streamNumber = streamIndex;
            streamNumber_00++;
        } while (streamNumber_00 < streamNumber);
    }
    gpRender->setStreamFrequency(streamNumber, 0x80000001);

    numInstances = 0;
    pMappedBuffer = nullptr;
    return false;
}
