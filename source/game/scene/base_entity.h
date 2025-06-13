#pragma once

#include "game/cinematic_entity.h"
#include "flash/mng_flash.h"

class Spot;
struct SoundScene;
class GameMechanism;

enum class eEntityType {
    ET_None = 0,          // 
    ET_321G = 858927431,  // 3, 2, 1... GO!
    ET_AACM = 1094796109, //
    ET_ACPA = 1094930497, //
    ET_ACTS = 1094931539, // Generic Actor
    ET_AIRC = 1095324227, // Aircraft
    ET_ANMS = 1095650643, // Animation Set
    ET_ASLB = 1095978050, // Lock Barrier
    ET_AVAT = 1096171860, // 
    ET_AVCO = 1096172367, // Avatar Convoy
    ET_AVHI = 1096173641, // Avatar HitchHiker
    ET_AVPL = 1096175692, // Avatar Driver (AVatar PLayer?)
    ET_AVSG = 1096176455, // Avatar ShoppingGirl
    ET_AVTR = 1096176722, // Avatar Transporter
    ET_BOAT = 1112490324, // Boat
    ET_BOPA = 1112494145, // 
    ET_BOTO = 1112495183, // 
    ET_CACI = 1128350537, // Camera Cinematic (?)
    ET_CARP = 1128354384, // Car Player
    ET_CHPT = 1128812628, // CheckPoint
    ET_CINE = 1128877637, // Cinematic
    ET_CPNJ = 1129336394, // NPC
    ET_CVOY = 1129729881, // Convoy
    ET_EVNT = 1163284052, // Event
    ET_FLAG = 1179402567, // Starting Line Flag
    ET_HIHI = 1212762185, // HitchHiker
    ET_HODA = 1213154369, // House Data
    ET_ITAC = 1229865283, //
    ET_INTE = 1229870149, //
    ET_LSET = 1280525652, // LifeSet
    ET_MICR = 1296646994, //
    ET_MUSD = 1297437508, // Music Description
    ET_PAGO = 1346455375, // Passenger Goodies
    ET_PDRS = 1346654803, //
    ET_POCA = 1347371841, // Pos Camera
    ET_PTDR = 1347699794, // Pedestrian
    ET_SDEM = 1396983117, // Sound Emitter
    ET_SDEN = 1396983118, //
    ET_SHDA = 1397244993, // Shop Data
    ET_SHOG = 1397247815, // ShoppingGirl
    ET_SPTM = 1397773389, // Spot Mesh
    ET_TRAN = 1414676814, // Transporter
    ET_TRLI = 1414679625, // Traffic Light
    ET_VELU = 1447382101, // Volume
    ET_WTHD = 1465141316, // Weather Desc
    ET_ZONE = 1515146821, // World Zone
    ET__ACT = 1598112596, // Actor
    ET__CAM = 1598243149, // Camera
    ET__ENT = 1598377556  // Entity
};

// TODO: Move to separate header
struct SerializedTransform {
    float RotationW = 1.0f;
    float RotationX = 0.0f;
    float RotationY = 0.0f;
    float RotationZ = 0.0f;
    float PositionX = 0.0f;
    float PositionY = 0.0f;
    float PositionZ = 0.0f;
    float ScaleX = 1.0f;
    float ScaleY = 1.0f;
    float ScaleZ = 1.0f;
};
static_assert(sizeof(SerializedTransform) == 40, "Size mismatch! Binary deserialization might fail" );

struct SerializedAttribute {
public:
    SerializedAttribute( void* pPtr )
        : pStreamPointer( pPtr )
    {
        
    }

    virtual bool serialize( void* pPtr ) { return true; }
    virtual bool deserialize( void* pPtr ) { return true; }
    virtual uint32_t getSize() const { return sizeof( void* ); }

protected:
    void* pStreamPointer;
};

struct SerializedMatrix : public SerializedAttribute {
    Eigen::Matrix4f* pMatrixPointer = nullptr;

    SerializedMatrix( Eigen::Matrix4f* pResource )
        : SerializedAttribute( pResource )
        , pMatrixPointer( pResource )
    {
        
    }

    virtual bool serialize( void* pPtr ) override 
    { 
        *(Eigen::Matrix4f*)pPtr = *pMatrixPointer;
        return true;
    }

    virtual bool deserialize( void* pPtr ) override 
    { 
        pMatrixPointer = (Eigen::Matrix4f*)pPtr;
        return true; 
    }

    virtual uint32_t getSize() const override { return sizeof( Eigen::Matrix4f ); }
};

struct EntityDesc {
    eEntityType         Type = eEntityType::ET__ENT;
    SerializedTransform Transform;
    float               Priority = 0.0f;
    float               RelevanceDistanceMax = 0.0f;
    uint16_t            Flags = 0;
};

class Entity : public CinematicEntity {
public:
    static const char* GetName() { return "Entity"; }

public:
    inline const Eigen::Matrix4f& getModelMatrix() const { return modelMatrix; }

public:
    Entity();
    ~Entity();

    virtual void initialize();
    virtual bool onMessage(FlashMessage& pMessage);

    void setTransform( Eigen::Matrix4f& transformMatrix, bool param_1 );

protected:
    static constexpr const FlashMessage kEnableParticles = FlashMessage( 0x70746f6e );
    static constexpr const FlashMessage kDisableParticles = FlashMessage( 0x70746f66 );

protected:
    std::string name;
    uint32_t flags;

    Eigen::Matrix4f modelMatrix;
    SerializedMatrix serializedModelMatrix;

    EntityDesc* pDesc;
    Entity* pParent;
    Spot* pSpot;
    SoundScene* pSound;
    GameMechanism* pMechanism;

    uint8_t bPrimaryInstance : 1;
    uint8_t bShifted : 1;

    std::vector<Entity*> children;
    int32_t defaultTileIndex;

protected:
    void setTransformFromDesc();
};
