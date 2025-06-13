#pragma once

class Entity;
class PhysicsDynamicController;

struct SerializedObject {
    uint64_t Hashcode;
    void* pPointer;
};

struct SerializedDynamicController : public SerializedObject {
    float FrictionDynamic;
    float FrictionStatic;
    float MaxClimbSlope;
};

struct SerializedPureDynamicController : public SerializedObject {
    SerializedObject Controlled;
    float	LinearDamping;
    float	AngularDamping;
    int32_t bStatic;
};

class GameMechanism {
public:
    GameMechanism();
    ~GameMechanism();

    void setEntity( Entity* pEntityInstance );

private:
    Eigen::Matrix4f initialTransformMatrix;
    Eigen::Matrix4f transformMatrix;
    
    PhysicsDynamicController* pController;
    PhysicsDynamicController* pParentController;

    SerializedDynamicController* pDynamicController;
    SerializedPureDynamicController* pPureDynamicController;

    float linearDamping;
    float angularDamping;

    Entity* pEntity;
    GameMechanism* pParent;
    int32_t numInstance;

    uint8_t bControlledByAvatar : 1;
    uint8_t bInitialized : 1;
    uint8_t bTransformSet : 1;
};

