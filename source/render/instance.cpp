#include "shared.h"
#include "instance.h"

#include "render/camera/camera_base.h"
#include "render/gs_render.h"

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
