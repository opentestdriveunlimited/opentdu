#pragma once

class CarState;

#include "vehicle_resource.h"
#include "render/light.h"
#include "render/setup_node.h"
#include "render/hiearchy_node.h"
#include "paint.h"
#include "filesystem/bank.h"
#include "physics/engine_simulation.h"
#include "render/particles/vehicle_particles.h"

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

struct VehicleSceneNode {
    Eigen::Matrix4f modelMatrix;
    Instance* pInstance;
    HiearchyNode* pSceneNode;
};

class VehicleBase {
public:
    static float FlareScaleStart;
    static float FlareScaleRatio;
    static float FlareZBiasRatio;
    static float FlareOldStartDistScale;
    static float FlareOldLengthDistScale;
    static float FlareOldScaleRatio;

    static bool bApplyWheelAttenuation;
    static float MinWheelAttenuation;

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

    enum class eFlareType {
        FT_Brake = 0,
        FT_BrakeFL = 1,
        FT_BrakeFR = 2,
        FT_BrakeRL = 3,
        FT_BrakeRR = 4,
        FT_BlinkL = 5,
        FT_BlinkR = 6,
        FT_Reverse = 7,
        FT_ReverseFL = 8,
        FT_ReverseFR = 9,
        FT_ReverseRL = 0xa,
        FT_ReverseRR = 0xb,
        FT_Headlights = 0xc,
        FT_HeadlightsRear = 0xd,
        FT_Warnings = 0xe,
        FT_HeadlightsFL = 0xf,
        FT_HeadlightsFR = 0x10,
        FT_HeadlightsRL = 0x11,
        FT_HeadlightsRR = 0x12,

        FT_Count
    };

public:
    VehicleBase();
    ~VehicleBase();

    virtual void initialize();
    void reset();
    void tick(float param_2);

    Instance* getInstanceByIndex(const uint32_t param_2);
    void setFlareValue(eFlareType param_2, const float param_3);

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
    
    LightSetupNode lightCfg;
    LightSetupNode lightCfgWheelL;
    LightSetupNode lightCfgWheelR;

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

    std::vector<VehicleSceneNode> childrenNodes;
    std::stack<Instance*> alphaInstances;

    VehicleParticles particles;

    uint32_t numMaterialInterior;
    uint32_t numColorInterior;
    uint32_t numColor2Interior;
    uint32_t numDynamicMaterial;

private:
    void updateLights();
};
