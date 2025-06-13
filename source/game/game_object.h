#pragma once

struct ObjectLocationRoad {
    uint32_t RoadID = 0;
    float AABBMinX = 0.0f;
    float AABBMinZ = 0.0f;
    float AABBMaxX = 0.0f;
    float AABBMaxZ = 0.0f;
};

struct ObjectLocationOther {
    uint32_t RoadID = 0;
    float PositionX = 0.0f;
    float PositionZ = 0.0f;
};

struct ObjectWorldLocation {
    float VelocityX;
    float VelocityZ;
    uint8_t LocationType : 2;
    uint8_t UnknownFlag : 1;
    uint8_t UnknownFlag2 : 1;

    union {
        ObjectLocationRoad LocationRoad;
        ObjectLocationOther LocationOther;
    };

    ObjectWorldLocation()
        : VelocityX( 0.0f )
        , VelocityZ( 0.0f )
        , LocationType( 0 )
        , UnknownFlag( 0 )
        , UnknownFlag2( 0 )
        , LocationRoad()
    {
        
    }
};

class P2PPlayer;

class GameObject {
public:
    GameObject();
    ~GameObject();

protected:
    uint32_t objectID;
    uint32_t parentID;
    uint32_t netID;
    float timeSinceLastUpdate;
    ObjectWorldLocation worldLocation;
    P2PPlayer* pNetOwner;
};
