#include "shared.h"
#include "weather_config.h"

#include "render/light.h"
#include "render/render_scene.h"

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

void WeatherConfig::fillDirectionalLight(DirectionalLight& light)
{
    light.Ambient = sunAmbientColor;
    light.Diffuse = sunDiffuseColor;
    light.Specular = sunSpecularColor;
    light.Direction = sunDirection;
}

void WeatherConfig::fillFogDesc(FogDesc& desc)
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
