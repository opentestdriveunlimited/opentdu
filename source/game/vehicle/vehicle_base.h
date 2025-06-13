#pragma once

class CarState;

#include "vehicle_resource.h"
#include "render/light.h"
#include "render/hiearchy_node.h"
#include "paint.h"
#include "filesystem/bank.h"
#include "physics/engine_simulation.h"

static constexpr uint32_t kMaxNumWheels = 4;

struct DBEntryRim;

struct VehicleBaseResources {
    Bank PhysicsBank;

    EngineSimulation* pEngineSim;
    std::vector<OmegaTorqueTuple> Torque;

    int32_t RefCount;
    int32_t CarID;
    int32_t TireID;
    int32_t RimID;
    uint64_t PackHashcode;

    uint8_t bIsLoading : 1;
    uint8_t bInitialized : 1;
    uint8_t bDefault : 1;
    uint8_t bSoundResource : 1;
    uint8_t bDriveResource : 1;
    uint8_t bEngineResource : 1;
};

class VehicleBase {
public:
    enum class eType {
        VBT_Car = 0,
        VBT_Bike = 1,
        VBT_TrafficCar = 2,
        VBT_TrafficBike = 3
    };

    enum class eLOD {
        VBL_Lvl0 = 0,
        VBL_Lvl1 = 1,
        VBL_Lvl2 = 2,
        VBL_Lvl3 = 3,
        VBL_Lvl4 = 4,
        VBL_Count
    };

    enum class eHeadlightState {
        VHS_Open = 0,
        VHS_Opening	= 1, // Popup lights only
        VHS_Closing	= 2, // Popup lights only
        VHS_Closed = 3	
    };

public:
    VehicleBase();
    ~VehicleBase();

private:
    CarState* pCarState;

    int32_t numHiearchyNodes;
    uint8_t bValidPosition : 1;
    uint8_t bLightsDirty : 1;
    uint8_t bInitialized : 1;
    uint8_t bShared : 1;
    uint8_t bShouldDraw : 1;
    uint8_t bVisible : 1;
    uint8_t bNeedToUnstream : 1;
    uint8_t bUseHQInterior : 1;
    uint8_t bUnknownFlag : 1;
    uint8_t bDisableShadow : 1;
    uint8_t bForceDrawShadow : 1;
    uint8_t bFirstRenderFrame : 1;
    uint8_t bPaintChange : 1;
    uint8_t bHasColorStrips : 1;
    uint8_t bUseTwoTonesPaint : 1;
    uint8_t bDrawReflections : 1;

    VehicleResource bodyResource;
    VehicleResource interiorResource;
    VehicleResource wheelsFront;
    VehicleResource wheelsRear;

    float currentEnvFactor;
    float currentEnvFactorBackFace;
    float currentLuminosity;

    eType type;
    eLOD currentLOD;

    std::array<float, kMaxNumWheels> pWheelOffsets;
    float armAngleOffset;
    float armLength;
    float dotFork;
    
    LightSetup lightCfg;
    LightSetup lightCfgWheelL;
    LightSetup lightCfgWheelR;

    DirectionalLight mainLight;
    DirectionalLight mainLightInterior;
    DirectionalLight wheelLILight;
    DirectionalLight wheelLLight;
    DirectionalLight wheelRILight;
    DirectionalLight wheelRLight;

    std::vector<HiearchyNodeInstance> displayInstances;

    HiearchyNode* pParentNode;
    HiearchyNode* pParentShadowNode;
    HiearchyNode* pParentShadowPlaneNode;

    int32_t numKits;
    int32_t currentKitIndex;
    
    std::vector<HiearchyNode*> kitHiearchyNodes;

    Instance* pVehicleBodyInstance;
    std::array<Instance*, kMaxNumWheels> vehicleWheelInstances;

    uint64_t hashGlassExterior;
    uint64_t hashGlassInterior;

    Material* pMaterialGlassExterior;
    Material* pMaterialGlassInterior;

    RenderFile* pShadowTexture;

    VehiclePaint paint;

    float alpha;
    int32_t alphaSlotIndex;
    std::stack<Instance*> alphaCache;
    uint8_t bAlphaCacheInUse : 1;

    DBEntryRim* pCurrentRimDB;
    DBEntryRim* pDefaultRimDB;
    float numSpokesRim; // TODO: Is it correct?

    std::vector<Material*> materials;
    std::vector<Material*> materialInstances;

    eHeadlightState headlightState;

    std::array<Instance*, 4> exhaustsDummies;

    std::vector<HiearchyNode*> flaresRL;
    std::vector<HiearchyNode*> flaresRR;
    std::vector<HiearchyNode*> flaresFL;
    std::vector<HiearchyNode*> flaresFR;
    std::vector<Instance*> flares;
};
