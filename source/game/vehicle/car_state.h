#pragma once

#include "game/game_object.h"
#include "game/material_types.h"
#include "physics/collision_event.h"

class DynamicRigidBody;

struct WheelState {
    float Omega;
    float SteerAngle;
    float CamberAngle;
    float CamberBreak;
    float SuspensionTravelLength;
    float SlipRatio;
    float SlipAngle;
    float LongitudinalSpeed;
    float LateralSpeed;
    float Load;
    float WheelAngle;
    float DistanceFromGround;
    float WheelRadius;
    float Position;
    int32_t WheelIndex;
    eMaterialType ContactMaterial;
    Eigen::Vector4f ShadowColor;
    Eigen::Vector4f GroundColor;
    Eigen::Vector4f RelativeWorldPosition;
    Eigen::Vector4f WorldPosition;
    Eigen::Matrix4f WorldMatrixRotor;
    Eigen::Matrix4f WorldMatrixFork;
    uint8_t bInitialized : 1;
    uint8_t bGrounded : 1;
    uint8_t bOnRoad : 1;
    uint8_t bOnBridge : 1;
    uint8_t bInsideTunnel : 1;
};


struct CarBodyState {
    Eigen::Matrix4f WorldMatrix;
    Eigen::Matrix4f OffsetedWorldMatrix;
    Eigen::Matrix4f PreviousWorldMatrix;
    Eigen::Matrix4f PhysicsWorldMatrix; // Looks like this is used to keep the Havok instance in sync with the "logic" scene instance. Remove it?
    
    Eigen::Vector4f AngularVelocity;
    Eigen::Vector4f LinearVelocity;
    Eigen::Vector4f Speed;
    Eigen::Vector4f SpeedDelta;

    float MaxOmega;
    float MaxTorque;
    float MinTorque;

    float Omega;
    float Torque;
    float SpeedInKmh;

    uint8_t bInitialized : 1;
    uint8_t bIsBraking : 1;
    uint8_t bIsReversing : 1;
    uint8_t bHeadlightsOn : 1;
    uint8_t bFlashingLights : 1;
    uint8_t bHazardLightsOn : 1;
    uint8_t bHonking : 1;
    uint8_t bIndicatingLeft : 1;
    uint8_t bIndicatingRight : 1;
    uint8_t bUnknown : 1;
    uint8_t bBackOnTracked : 1;
    uint8_t bUseWheelMatrix : 1;
    uint8_t bIsInTunnel : 1;
    uint8_t bHasBrokenPart : 1; // Seems only to be used for traffic?
    uint8_t bIsBike : 1;
    uint8_t bBikeIdle : 1;
    uint8_t bHasBikeAvatar : 1;

    int32_t WindowState;
    int32_t Gears;
    int32_t GearState;
    int32_t OdometerKms;
    int32_t OdometerTenMeters;

    float Distance;
    int32_t NumWheelsOnRoad;
    int32_t NumWheelsGrounded;

    float GearShiftTimer;
    float BikeLeanLateral;
    float BikeLeanLongitudinal;
    float SteeringRaw;

    // Inputs.
    float Steering;
    float Throttle;
    float Brake;

    Eigen::Vector4f Shaking;
    Eigen::Vector4f BikeVelocity;
    Eigen::Matrix4f WorldBikeMatrix;
    Eigen::Matrix4f BikeHelmetMatrix;

    Eigen::Vector4f PositionOnRoad;
    Eigen::Vector4f PrevPositionOnRoad;
    int32_t ActiveRoadID;
    float RoadX;
    float RoadZ;

    int32_t BackOnTrackState;
    float BackOnTrackTimer;
    Eigen::Matrix4f BackOnTrackWorldMatrix;
    Eigen::Vector4f BackOnTrackVelocity;
    
    Eigen::Vector4f BodyDimensions;
    float WheelBase;
    float TrackWidth;
    float TrackLength;

    std::string Name;
    std::string NamePhysicsWorld;

    std::vector<CollisionEvent> CollisionEvents;
};

class CarState : public GameObject {
public:
    enum class eOwner {
        CSO_LocalPlayer = 0,
        CSO_NetPlayer = 1,
        CSO_Traffic = 2,
        CSO_Bot = 3
    };

    enum class eSimulationType {
        CSST_ReplicatedFromNetwork = 0,
        CSST_Local = 1,
        CSST_None = 2 // Seems to be used for cutscenes. Could actually be some keyframed mode (or some cutscene specific mode)?
    };

    enum class eActiveMode {
        CSAM_Paused = 0,
        CSAM_Idle = 1,
        CSAM_Freeride = 2,
        CSAM_TimeAttack = 3,
        CSAM_InstantChallengeAsk = 4,
        CSAM_InstantChallengeRefuse = 5,
        CSAM_InstantChallengePreLobby = 6,
        CSAM_InstantChallenge = 7,
        CSAM_InstantChallengePostLobby = 8
    };

    using OnCollisionCallback_t = std::function<void(uint32_t, CarState*, DynamicRigidBody*, void* )>;

public:
    CarState();
    ~CarState();

private:
    static constexpr const uint32_t kMaxNumWheels = 4;

private:
    std::string name;

    CarState::eOwner owner;
    CarState::eSimulationType simulationType;
    CarState::eActiveMode activeMode;

    int32_t carInstanceID;
    uint32_t numWheels;
    CarBodyState body;
    std::array<WheelState, kMaxNumWheels> wheels;

    int32_t carID;
    int32_t tireID;
    int32_t rimID;
    int32_t colorID;

    uint8_t bInstanceAlive : 1;
    uint8_t bIsIndoor : 1;
    uint8_t bUseShowcaseControl : 1;

    uint64_t packHashcode;
};
