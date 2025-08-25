#include "shared.h"
#include "vehicle_base.h"

#include "world/weather/gs_weather.h"
#include "car_state.h"
#include "mng_car_alpha.h"

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

    flaresRL.fill( nullptr );
    flaresRR.fill( nullptr );
    flaresFL.fill( nullptr );
    flaresFL.fill( nullptr );
    flares.fill( nullptr );

    headlightState = eHeadlightState::VHS_Open;

    pVehicleBodyInstance = nullptr;
    vehicleWheelInstances.fill( nullptr );
    
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
    // FUN_008c0450
    particles.update(this);
    updateLights();

    float fVar2 = 0.0f;
    if (bLightsDirty && !bFirstRenderFrame && type != eType::VBT_TrafficCar) {
        fVar2 = pCarState->isReversing() ? 1.0f : 0.0f;
        setFlareValue(eFlareType::FT_Reverse, fVar2);

        fVar2 = pCarState->isBraking() ? 1.0f : 0.0f;
        setFlareValue(eFlareType::FT_Brake, fVar2);

        fVar2 = pCarState->isIndicatingLeft() ? 1.0f : 0.0f;
        setFlareValue(eFlareType::FT_BlinkL, fVar2);

        fVar2 = pCarState->isIndicatingRight() ? 1.0f : 0.0f;
        setFlareValue(eFlareType::FT_BlinkR, fVar2);

        fVar2 = (!pCarState->headlightsOn() || headlightState != eHeadlightState::VHS_Open) ? 0.0f : 1.0f; 
        setFlareValue(eFlareType::FT_Headlights, fVar2);
        setFlareValue(eFlareType::FT_HeadlightsRear, 0.0f);

        fVar2 = pCarState->hazardLightsOn() ? 1.0f : 0.0f;
        setFlareValue(eFlareType::FT_Warnings, fVar2);

        bLightsDirty = false;
    }
    bFirstRenderFrame = false;

    if (!displayInstances.empty() && !bNeedToUnstream && gpMngCarAlpha != nullptr && bValidPosition) {
        updateCarAlpha();
    }
}

void VehicleBase::setFlareValue(VehicleBase::eFlareType param_2, const float param_3)
{
    // FUN_008bb7a0
    uint8_t bVar6 = 0;
    int32_t local_6c = 0;

    TestDriveString8 local_58[10];

    if (type == eType::VBT_TrafficCar) {
        bVar6 = 1;
        switch (param_2) {
        case eFlareType::FT_Brake:
            local_58[0] = "P_FL";
            local_58[3] = "P_FL";
            local_58[7] = "P_FR";
            local_58[8] = "P_RR";
            local_6c = 2;
            break;
        case eFlareType::FT_BrakeFL:
        case eFlareType::FT_BrakeRL:
            local_58[0] = "P_FL";
            local_6c = 2;
            break;
        case eFlareType::FT_BrakeFR:
            local_58[0] = "P_FR";
            local_6c = 2;
            break;
        case eFlareType::FT_BrakeRR:
            local_58[0] = "P_RR";
            local_6c = 2;
            break;
        case eFlareType::FT_BlinkL:
            local_58[1] = "LGT_L";
            local_58[0] = "P_FL";
            local_58[3] = "P_FL";
            local_6c = 3;
            break;
        case eFlareType::FT_BlinkR:
            local_58[1] = "LGT_R";
            local_58[0] = "P_FR";
            local_58[3] = "P_RR";
            local_6c = 3;
            break;
        case eFlareType::FT_Reverse:
            local_58[1] = "LGT_L";
            local_58[2] = "LGT_R";
            local_58[0] = "P_FL";
            local_58[3] = "P_FL";
            local_58[7] = "P_FR";
            local_58[8] = "P_RR";
            local_6c = 4;
            break;
        case eFlareType::FT_ReverseFL:  
            local_58[0] = "P_FL";
            local_6c = 4;
            break;
        case eFlareType::FT_ReverseFR:  
            local_58[0] = "P_FR";
            local_6c = 4;
            break;
        case eFlareType::FT_ReverseRL:  
            local_58[0] = "P_FL";
            local_6c = 4;
            break;
        case eFlareType::FT_ReverseRR:  
            local_58[0] = "P_RR";
            local_6c = 4;
            break;
        case eFlareType::FT_Headlights:
            local_58[1] = "LGT_L";
            local_58[2] = "LGT_R";
            local_58[0] = "P_FL";
            local_58[3] = "P_FR";
            local_6c = 0;
            break;
        case eFlareType::FT_HeadlightsRear:
            local_58[1] = "LGT_L";
            local_58[2] = "LGT_R";
            local_58[0] = "P_FL";
            local_58[3] = "P_RR";
            local_6c = 2;
            break;
        case eFlareType::FT_Warnings:
            local_58[0] = "P_FL";
            local_58[3] = "P_FL";
            local_58[7] = "P_FR";
            local_58[8] = "P_RR";
            local_6c = 1;
            break;
        case eFlareType::FT_HeadlightsFL:
            local_58[0] = "P_FL";
            local_6c = 0;
            break;
        case eFlareType::FT_HeadlightsFR:
            local_58[0] = "P_FR";
            local_6c = 0;
            break;
        case eFlareType::FT_HeadlightsRL:
            local_58[0] = "P_FL";
            local_6c = 0;
            break;
        case eFlareType::FT_HeadlightsRR:
            local_58[0] = "P_RR";
            local_6c = 0;
            break;
        default:
            OTDU_UNIMPLEMENTED;
            break;
        }
    } else {
        bVar6 = 0;
        switch (param_2) {
        case eFlareType::FT_Brake:
            local_58[1] = "LGT_RL";
            local_58[2] = "LGT_FL";
            local_58[5] = "LGT_FR";
            local_58[6] = "LGT_RR";
            local_6c = 2;
            break;
        case eFlareType::FT_BlinkL:
            local_58[1] = "LGT_RL";
            local_58[2] = "LGT_FL";
            local_6c = 3;
            break;
        case eFlareType::FT_BlinkR:
            local_58[1] = "LGT_FR";
            local_58[2] = "LGT_RR";
            local_6c = 3;
            break;
        case eFlareType::FT_Reverse:
            local_58[1] = "LGT_RL";
            local_58[2] = "LGT_FL";
            local_58[5] = "LGT_FR";
            local_58[6] = "LGT_RR";
            local_6c = 4;
            break;
        case eFlareType::FT_Headlights:
            local_58[1] = "LGT_FR";
            local_58[2] = "LGT_FL";
            local_6c = 0;
            break;
        case eFlareType::FT_HeadlightsRear:
            local_58[1] = "LGT_RR";
            local_58[2] = "LGT_RL";
            local_6c = 3;
            break;
        case eFlareType::FT_Warnings:
            local_58[1] = "LGT_RL";
            local_58[2] = "LGT_FL";
            local_58[5] = "LGT_FR";
            local_58[6] = "LGT_RR";             
            local_6c = 1;
            break;
        }
    }
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

void VehicleBase::updateCarAlpha()
{
    // FUN_008bf510
    if ((1.0f < alpha || alpha == 1.0f) && (alphaSlotIndex != 0xffffffff)) {
        registerCarAlphaInstances(alphaSlotIndex, false);
        gpMngCarAlpha->freeElement(alphaSlotIndex);
        alphaSlotIndex = 0xffffffff;
    }
}

void VehicleBase::registerCarAlphaInstances(uint32_t param_2, bool param_3)
{
    // FUN_008babc0
    if (gpMngCarAlpha == nullptr) {
        return;
    }

    if (!bAlphaCacheInUse) {
        buildCarAlphaInstances();
    }

    if (bAlphaCacheInUse && !alphaInstances.empty()) {
        for (Instance* peVar2 : alphaInstances) {
            RenderFile::Section* peVar3 = peVar2->getMaterialBank();
            if (peVar3 != nullptr) {
                uint32_t uVar8 = ( peVar3->Size - 0x10 ) / sizeof( Render2DM::HashTableEntry );
                Render2DM::HashTableEntry* pIterator = ( Render2DM::HashTableEntry* )( peVar3 + 1 );

                for ( uint32_t i = 0; i < uVar8; i++ ) {
                    if (pIterator->pMaterial != nullptr) {
                        Material* peVar1 = (Material*)(pIterator->pMaterial + 1);
                        gpMngCarAlpha->registerMaterial(param_2, peVar1, param_3);
                    }
                    pIterator++;
                }
            }
        }
    }
    
    for (Instance* peVar2 : flares) {
        if (peVar2 != nullptr) {
            if (param_3) {
                peVar2->getFlagsWrite() = peVar2->getFlagsWrite() | 0x10;
            } else {
                peVar2->getFlagsWrite() = peVar2->getFlagsWrite() & 0xffffffef;
            }
        }
    }

    if (pParentShadowPlaneNode != nullptr) {
        Instance* peVar2 = pParentShadowPlaneNode->pInstance;
        if (param_3) {
            peVar2->getFlagsWrite() = peVar2->getFlagsWrite() | 0x10;
            return;
        }
        peVar2->getFlagsWrite() = peVar2->getFlagsWrite() & 0xffffffef;
    }
}

void VehicleBase::buildCarAlphaInstances()
{
    // FUN_008b9d40
    alphaInstances.clear();

    if (displayInstances.empty() || bNeedToUnstream) {
        bAlphaCacheInUse = true;
        return;
    }
    
    HiearchyNode* peVar4 = nullptr;
    if (!bNeedToUnstream) {
        peVar4 = displayInstances[0].pNode;
    } else {
        peVar4 = nullptr;
    }

    if (peVar4 == nullptr) {
        bAlphaCacheInUse = true;
        return;
    }

    do {
        Instance* peVar1 = peVar4->pInstance;
        if (peVar1 != nullptr) {
            bool bVar3 = true;

            for (uint32_t iVar6 = 0; iVar6 < 0x3c; iVar6++) {
                uint32_t ppeVar5 = 1 + iVar6 * 6;
                if (!bVar3 || peVar1 == flares[ppeVar5 - 1] || peVar1 == flares[ppeVar5] 
                           || peVar1 == flares[ppeVar5 + 1] || peVar1 == flares[ppeVar5 + 2]
                           || peVar1 == flares[ppeVar5 + 3]) {
                    bVar3 = false;
                    break;
                }

                if (peVar1 == flares[ppeVar5 + 4]) {
                    bVar3 = false;
                }
            }

            if (bVar3) {
                alphaInstances.push_front(peVar1);
            }
        }
        peVar4 = peVar4->pNext;
        if (peVar4 == nullptr) {
            bAlphaCacheInUse = true;
            return;
        }
    } while ( true );
}
