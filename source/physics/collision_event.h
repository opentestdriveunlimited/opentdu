#pragma once

#include "game/material_types.h"
#include "game/game_object.h"

enum class eCollisionType {
    CT_Car = 0,
    CT_RigidBody = 1,
    CT_DynamicBody = 2
};

class PhysicsVehicle;
class PhysicsStaticRigidBody;
class PhysicsDynamicRigidBody;

struct CollisionEvent {
    int32_t EventID;
    eCollisionType Type;
    uint8_t bUsed : 1;

    union {
        PhysicsVehicle*             pOtherVehicle;
        PhysicsStaticRigidBody*     pStaticRigidBody;
        PhysicsDynamicRigidBody*    pDynamicRigidBody;
    };

    float RelativeVelocity;
    float Energy;

    eMaterialType CarMaterial;
    eMaterialType OtherMaterial;
    
    Eigen::Vector4f ContactPosition;
    Eigen::Vector4f Velocity;

    ObjectWorldLocation Location;
};
