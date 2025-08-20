#include "shared.h"
#include "weather_config.h"

#include "render/light.h"
#include "render/render_scene.h"
#include "gs_timer.h"

static float DAT_016a6a30 = 0.0f;
static float DAT_00fab8d4 = 0.1f;

WeatherConfig::WeatherConfig(const char *pName, const bool bHDR)
    : name( pName ? pName : "WEATHER_CFG_UNKNOWN" )
    , globalAmbientColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , sunAmbientColor( 0.4f, 0.4f, 0.4f, 1.0f )
    , sunDiffuseColor( 0.8f, 0.8f, 0.8f, 1.0f )
    , sunSpecularColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , sunDirection( -0.4f, 0.0f, -1.0f)
    , windDirection( 0.0f, 1.0f, 0.0f, 0.0f )
    , windSpeed( 0.0f )
    , noisePeriod( 5.0f )
    , noisePeriodTemporal( 2.50f )   
    , maxWindSpeed( 5.0f )
    , maxWindSpeedRandomized( 0.80f )
    , leafWindSpeed( 1.5f )
    , cloudsColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , cloudsScaleUV( 2.5f, 2.5f )
    , cloudsCover( 0.50f )
    , cloudsSharpness( 0.05f )
    , cloudsPacifier( 1.0f )
    , cloudsAlphaStart( 90000.0f )
    , cloudsAlphaLength( 20000.0f )
    , fogRange( 0.0f, 12500.0f )
    , fogMaxIntensity( 1.0f )
    , fogColor( 0.39f, 0.58f, 0.79f, 1.0f )
    , fogColorAdditive( 1.0f, 1.0f, 1.0f, 1.0f )
    , sunCoreColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , sunHaloColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , sunCoef( 0.2f )
    , sunCoefHalo( 0.4f )
    , sunMarkerSize( 0.05f )
    , sunDazzleDecreaseSpeed( 0.5f )
    , sunDazzleThreshold( 0.6f )
    , oceanAmplitude( 6.0f )
    , skydome2DBName( "dome" )
    , skydomeEmissiveColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , cloudsEmissiveFactor( 1.0f )
    , sumEmissiveFactor( 1.0f )
    , filter2DBName( "" )
    , sunBillboard2DBName( "" )
    , sunHalo2DBName( "" )
    , terrainColor( 1.0f, 1.0f, 1.0f, 1.0f )
    , terrainContrast( 1.0f )
    , contrast( 1.0f )
    , desaturation( 1.0f )
    , balance( Eigen::Vector3f::Zero() )
    , carLuminanceRange( 0.0f, 1.0f )
    , carGlobalFactor( 100.0f )
    , carEnvFactor( 0.70f )
    , carAmbiantFactor( 0.50f )
    , carDiffuseFactor( 1.0f )
    , carSpecularFactor( 0.70f )
    , carReflectionFactor( 0.0f )
    , carReflectionLuminosity( 1.2f )
    , carCloudsFactor( 0.2f )
    , pHDRIConfig( nullptr )
    , bHDRIEnabled( bHDR )
{

}

WeatherConfig::~WeatherConfig()
{
}

void WeatherConfig::fillDirectionalLight(DirectionalLight& light) const
{
    light.Ambient = sunAmbientColor;
    light.Diffuse = sunDiffuseColor;
    light.Specular = sunSpecularColor;
    light.Direction = sunDirection;
}

void WeatherConfig::fillFogDesc(FogDesc& desc) const
{
    desc.Color = fogColor;
    desc.DistanceStart = fogRange.x();
    desc.DistanceEnd = fogRange.y();
    desc.Intensity = fogMaxIntensity;
    desc.Type = eFogType::FT_Linear;
}

Eigen::Vector4f WeatherConfig::getGlobalAmbientColor() const
{
    return globalAmbientColor;
}

Eigen::Vector4f WeatherConfig::getTerrainUniformParams() const
{
    return Eigen::Vector4f( terrainColor.x(), terrainColor.y(), terrainColor.z(), terrainContrast);
}

Eigen::Vector3f WeatherConfig::getSunDirection() const
{
    return sunDirection;
}

float WeatherConfig::getWindSpeed() const
{
    return windSpeed;
}

float WeatherConfig::getTreeWindSpeed(float param_2, Eigen::Vector4f &param_3) const
{
    float fVar1 = sin((param_2 / noisePeriodTemporal) * 6.2831855f
                        * gGSTimer.GameTotalTime +
                        (param_2 / noisePeriod) * 6.2831855f
                        * param_3.norm());
    return (fVar1 * maxWindSpeedRandomized + 1.0f) * windSpeed;
}

Eigen::Vector3f WeatherConfig::getRandomizedWindDirection(float param_1, Eigen::Vector4f &param_3) const
{
    float fVar1 = sin((param_1 / noisePeriodTemporal) * 6.2831855f
                        * gGSTimer.GameTotalTime +
                        (param_1 / noisePeriod) * 6.2831855f
                        * param_3.norm());
    fVar1 = fVar1 * maxWindSpeed * 0.0055555557f * 0.31830987f;
    float fVar2 = cos(fVar1);
    fVar1 = sin(fVar1);

    Eigen::Vector3f param_4 = {
        (fVar2 * windDirection.x() + fVar1 * windDirection.z()),
        0.0f,
        (fVar2 * windDirection.z() - fVar1 * windDirection.x())
    };
    return param_4;
}

float WeatherConfig::getRandomizedLeafWindSpeed(Eigen::Vector4f &param_3) const
{ 
    float fVar1 = sin((6.2831855f / noisePeriodTemporal) *
                        gGSTimer.GameTotalTime +
                        (6.2831855f / noisePeriod) 
                        * param_3.norm());
    return (fVar1 * DAT_016a6a30 + 1.0f) * windSpeed * leafWindSpeed * DAT_00fab8d4;
}
