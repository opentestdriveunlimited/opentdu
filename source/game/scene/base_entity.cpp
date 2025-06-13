#include "shared.h"
#include "base_entity.h"

#include "game/game_mechanism.h"

Entity::Entity()
    : CinematicEntity()
    , name( "Entity" )
    , flags( 0x7 )
    , modelMatrix( Eigen::Matrix4f::Identity() )
    , serializedModelMatrix( &modelMatrix )
    , pDesc( nullptr )
    , pParent( nullptr )
    , pSpot( nullptr )
    , pSound( nullptr )
    , pMechanism( nullptr )
    , bPrimaryInstance( false )
    , bShifted( false )
    , defaultTileIndex(0)
{
    
}

Entity::~Entity()
{

}

void Entity::initialize()
{
    setTransformFromDesc();

    if (pMechanism != nullptr) {
        pMechanism->setEntity(this);
    }
}

bool Entity::onMessage(FlashMessage &pMessage)
{
    switch (pMessage) {
        case Entity::kEnableParticles: {
            return true;
        }
        case Entity::kDisableParticles: {
            return true;
        }
        default:
            OTDU_LOG_WARN("Unknown flash message '%s' (%x)\n", pMessage.pAsChar, pMessage.AsInteger);
            break;
    }

    return true;
}

void Entity::setTransform(Eigen::Matrix4f &transformMatrix, bool param_1)
{
    bShifted = !param_1;
    modelMatrix = transformMatrix;
}

void Entity::setTransformFromDesc()
{
    OTDU_ASSERT(pDesc);
    
    Eigen::Quaternionf rotation(pDesc->Transform.RotationW, pDesc->Transform.RotationX, pDesc->Transform.RotationY, pDesc->Transform.RotationZ);
    Eigen::Matrix3f rotationMatrix = rotation.toRotationMatrix();
    Eigen::Matrix4f transformMatrix;
    transformMatrix << 
        rotationMatrix.row(0), 
        rotationMatrix.row(1), 
        rotationMatrix.row(2), 
        Eigen::Vector4f(pDesc->Transform.PositionX, pDesc->Transform.PositionY, pDesc->Transform.PositionZ, 1.0f);

    setTransform(transformMatrix, true);
}
