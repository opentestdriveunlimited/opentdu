#include "shared.h"
#include "vehicle_base.h"

#include "world/weather/gs_weather.h"
#include "car_state.h"

static const uint64_t kHashcodeForkFL = GetIdentifier64bit("FORK_FL");
static const uint64_t kHashcodeForkFR = GetIdentifier64bit("FORK_FR");
static const uint64_t kHashcodeForkRL = GetIdentifier64bit("FORK_RL");
static const uint64_t kHashcodeForkRR = GetIdentifier64bit("FORK_RR");

float VehicleBase::FlareScaleStart = 0.0f;
float VehicleBase::FlareScaleRatio = 0.0f;
float VehicleBase::FlareZBiasRatio = 0.0f;
float VehicleBase::FlareOldStartDistScale = 0.0f;
float VehicleBase::FlareOldLengthDistScale = 0.0f;
float VehicleBase::FlareOldScaleRatio = 0.0f;

// TODO: I am not sure if VehicleBase is supposed to own those or if those are globals...
bool VehicleBase::bApplyWheelAttenuation = true; // DAT_00f7f198
float VehicleBase::MinWheelAttenuation = 0.2f; // DAT_00f7f194

VehicleBase::VehicleBase()
    : pCarState( nullptr )
    , numHiearchyNodes( 0 )
    , bValidPosition( false )
    , bLightsDirty( false )
    , bInitialized( false )
    , bShared( false )
    , bShouldDraw( false )
    , bVisible( false )
    , bNeedToUnstream( false )
    , bUseHQInterior( false )
    , bUnknownFlag( false )
    , alpha( 1.0f )
    , alphaSlotIndex( -1 )
    , bAlphaCacheInUse( false )
    , numMaterialInterior( 0 )
    , numColorInterior( 0 )
    , numColor2Interior( 0 )
    , numDynamicMaterial( 0 )
{
    
}

VehicleBase::~VehicleBase()
{
}

void VehicleBase::initialize()
{
    if (gpWeather->isConfigDirty()) {
        gpWeather->updateActiveConfig();
    }

    // Exterior
    gpWeather->getActiveWeatherConfig().fillDirectionalLight(mainLight);
    gpWeather->getActiveWeatherConfig().fillDirectionalLight(mainLightInterior);
    lightCfg.addLight(&mainLight);
    lightCfg.addLight(&mainLightInterior);

    // Wheels (left)
    wheelLLight = mainLight;
    wheelLILight = mainLightInterior;

    lightCfgWheelL.addLight(&wheelLLight);
    lightCfgWheelL.addLight(&wheelLILight);

    // Wheels (right)
    wheelRLight = mainLight;
    wheelRILight = mainLightInterior;

    lightCfgWheelR.addLight(&wheelRLight);
    lightCfgWheelR.addLight(&wheelRILight);

    currentEnvFactor = 1.0f;
    currentEnvFactorBackFace = 0.1f;
    pWheelOffsets[3] = 0.0f;
    armAngleOffset = 0.0f;
    armLength = 0.0f;
    dotFork = 0.0f;
}

void VehicleBase::reset()
{
    bInitialized = false;
    bValidPosition = false;
    bShared = false;
    bVisible = true;
    bShouldDraw = true;

    pParentNode = nullptr;
    pParentShadowNode = nullptr;
    pParentShadowPlaneNode = nullptr;

    pCarState = nullptr;

    numSpokesRim = 5.0f;

    bNeedToUnstream = false;
    bUnknownFlag = false;
    bDisableShadow = false;
    bForceDrawShadow = false;

    pShadowTexture = nullptr;

    for (VehicleSceneNode& node : childrenNodes) {
        node.pSceneNode = nullptr;
        node.pInstance = nullptr;
    }

    numKits = 0;
    currentKitIndex = 0;
    kitHiearchyNodes.clear();

    pMaterialGlassExterior = nullptr;
    pMaterialGlassInterior = nullptr;

    bodyResource.pResourceBank = nullptr;
    bodyResource.pResource2DM = nullptr;
    bodyResource.pResource3DD = nullptr;
    
    interiorResource.pResourceBank = nullptr;
    interiorResource.pResource2DM = nullptr;
    interiorResource.pResource3DD = nullptr;
    
    particles.reset();

    pCurrentRimDB = nullptr;
    bHasColorStrips = false;
    bUseTwoTonesPaint = false;

    numMaterialInterior = 0;
    numColorInterior = 0;
    numColor2Interior = 0;
    numDynamicMaterial = 0;

    VehicleBase::FlareScaleStart = 20.0f;
    VehicleBase::FlareScaleRatio = 25.0f;
    VehicleBase::FlareZBiasRatio = 3.0f;
    VehicleBase::FlareOldStartDistScale = 0.0f;
    VehicleBase::FlareOldLengthDistScale = 0.0f;
    VehicleBase::FlareOldScaleRatio = 0.0f;

    bDrawReflections = false;
    bPaintChange = false;

    flaresRL.clear();
    flaresRR.clear();
    flaresFL.clear();
    flaresFL.clear();
    flares.clear();

    headlightState = eHeadlightState::VHS_Open;

    pVehicleBodyInstance = nullptr;
    for ( Instance* pWheel : vehicleWheelInstances ) {
        pWheel = nullptr;
    }
    
    // TODO: Check what's being reset here (offset points to a dword)
    // 008ba785 88 9a 2c        MOV        byte ptr [EDX + 0x1e2c],BL

    paint.clear();
}

Instance* VehicleBase::getInstanceByIndex(const uint32_t param_2)
{
    if (bNeedToUnstream) {
        return nullptr;
    }

    OTDU_ASSERT( param_2 < displayInstances.size() );
    return displayInstances[param_2].pInstance;
}

void VehicleBase::tick(float param_2)
{
    particles.update(this);
    updateLights();

    if (bLightsDirty && !bFirstRenderFrame && type != eType::VBT_TrafficCar) {
        OTDU_UNIMPLEMENTED;
    }
}

void VehicleBase::setFlareValue(VehicleBase::eFlareType param_2, const float param_3)
{
   OTDU_UNIMPLEMENTED;
}

void VehicleBase::updateLights()
{
    if (type == eType::VBT_Car || type == eType::VBT_TrafficCar) {
        wheelLLight = mainLight;
        wheelRLight = mainLight;

        wheelLILight = mainLightInterior;
        wheelRILight = mainLightInterior;

        if (bApplyWheelAttenuation) {
            gpWeather->updateActiveConfig();

            Eigen::Vector3f local_30 = gpWeather->getActiveWeatherConfig().getSunDirection();
            local_30.normalize();

            Eigen::Vector3f local_20 = pCarState->getWorldPosition();

            // Call to FUN_00408070 (0x008c01ad) implicitly set w to 0.0f.
            // We do it inline since we are not re-using their custom math lib
            Eigen::Vector4f pTVar2 = Eigen::Vector4f(local_20.x(), 0.0f, local_20.z(), 0.0f);
            pTVar2.normalize();

            DirectionalLight* pLightOut = &wheelLLight;
            DirectionalLight* pLightInt = &wheelLILight;

            float fVar8 = pTVar2.z() * local_30.z() + pTVar2.y() * local_30.y() + pTVar2.x() * local_30.x();
            if (-0.01 <= fVar8) {
                if (fVar8 <= 0.01) {
                    return;
                }
            } else {
                fVar8 = 0.0f - fVar8;

                pLightOut = &wheelRLight;
                pLightInt = &wheelRILight;
            }
            float fVar9 = MinWheelAttenuation;
            if (pCarState != nullptr && pCarState->isIndoor()) {
                fVar9 = MinWheelAttenuation * 4.0f;
            }
            fVar9 = (1.0f - fVar9) * (1.0f - fVar8) + fVar9;

            (pLightOut->Specular).x() = (pLightOut->Specular).x() * fVar9;
            (pLightOut->Specular).y() = (pLightOut->Specular).y() * fVar9;
            (pLightOut->Specular).z() = (pLightOut->Specular).z() * fVar9;
            (pLightOut->Specular).w() = (pLightOut->Specular).w();
            (pLightOut->Diffuse).x() = (pLightOut->Diffuse).x() * fVar9;
            (pLightOut->Diffuse).y() = (pLightOut->Diffuse).y() * fVar9;
            (pLightOut->Diffuse).z() = (pLightOut->Diffuse).z() * fVar9;
            (pLightOut->Diffuse).w() = (pLightOut->Diffuse).w();

            (pLightInt->Specular).x() = (pLightInt->Specular).x() * fVar9 * 0.5f;
            (pLightInt->Specular).y() = (pLightInt->Specular).y() * fVar9 * 0.5f;
            (pLightInt->Specular).z() = (pLightInt->Specular).z() * fVar9 * 0.5f;
            (pLightInt->Specular).w() = (pLightInt->Specular).w();
            (pLightInt->Diffuse).x() = (pLightInt->Diffuse).x() * fVar9 * 0.5f;
            (pLightInt->Diffuse).y() = (pLightInt->Diffuse).y() * fVar9 * 0.5f;
            (pLightInt->Diffuse).z() = (pLightInt->Diffuse).z() * fVar9 * 0.5f;
            (pLightInt->Diffuse).w() = (pLightInt->Diffuse).w();
        }
    }
}
