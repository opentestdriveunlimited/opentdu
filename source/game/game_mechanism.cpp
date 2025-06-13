#include "shared.h"
#include "game_mechanism.h"

#include "game/scene/base_entity.h"

GameMechanism::GameMechanism()
    : initialTransformMatrix( Eigen::Matrix4f::Identity() )
    , transformMatrix( Eigen::Matrix4f::Identity() )
    , pController( nullptr )
    , pParentController( nullptr )
    , pDynamicController( nullptr )
    , pPureDynamicController( nullptr )
    , linearDamping( 0.0f )
    , angularDamping( 0.05f )
    , pEntity( nullptr )
    , pParent( nullptr )
    , numInstance( 0u )
    , bControlledByAvatar( false )
    , bInitialized( false )
    , bTransformSet( false )
{
    
}

GameMechanism::~GameMechanism()
{
    
}

void GameMechanism::setEntity(Entity *pEntityInstance)
{
    pEntity = pEntityInstance;

    if (pParent == nullptr && !bTransformSet && !bControlledByAvatar) {
        Eigen::Matrix4f* pTVar3 = &initialTransformMatrix;
        const Eigen::Matrix4f& pTVar2 = pEntityInstance->getModelMatrix();
        Eigen::Matrix4f* pTVar4 = pTVar3;

        initialTransformMatrix = pEntityInstance->getModelMatrix();
        transformMatrix = initialTransformMatrix;

        Eigen::Matrix4f local_50 = initialTransformMatrix.inverse();

        OTDU_UNIMPLEMENTED; // TODO: implement setWorldMatrix
        // pController->setWorldMatrix(local_50);
        
        bTransformSet = true;
    }
}
