#pragma once

#include "hdri_config.h"
#include "render/light.h"
#include "render/render_scene.h"

class WeatherConfig {
public:
    static constexpr const char* kDefaultName       = "DEFAULT";
    static constexpr const char* kDefaultNameHDR    = "HDR_DEFAULT";

public:
    WeatherConfig(const char* pName = nullptr, const bool bHDR = false);
    ~WeatherConfig();

    void fillDirectionalLight(DirectionalLight& light) const;
    void fillFogDesc(FogDesc& desc) const;
    
    Eigen::Vector4f getGlobalAmbientColor() const;
    Eigen::Vector4f getTerrainUniformParams() const;
    Eigen::Vector3f getSunDirection() const;

private:
    std::string name;

    Eigen::Vector4f globalAmbientColor;
    Eigen::Vector4f sunAmbientColor;
    Eigen::Vector4f sunDiffuseColor;
    Eigen::Vector4f sunSpecularColor;
    Eigen::Vector3f sunDirection;
    Eigen::Vector4f windDirection;
    
    float windSpeed;
    float noisePeriod;
    float noisePeriodTemporal;
    float maxWindSpeed;
    float maxWindSpeedRandomized;
    float leafWindSpeed;

    Eigen::Vector4f cloudsColor;
    Eigen::Vector2f cloudsScaleUV;
    float           cloudsCover;
    float           cloudsSharpness;
    float           cloudsPacifier;
    float           cloudsAlphaStart;
    float           cloudsAlphaLength;

    std::vector<std::string> cloudsNames;

    Eigen::Vector2f fogRange;
    float fogMaxIntensity;
    Eigen::Vector4f fogColor;
    Eigen::Vector4f fogColorAdditive;

    Eigen::Vector4f sunCoreColor;
    Eigen::Vector4f sunHaloColor;
    float sunCoef;
    float sunCoefHalo;
    float sunMarkerSize;
    float sunDazzleDecreaseSpeed;
    float sunDazzleThreshold;

    float oceanAmplitude;

    std::string skydome2DBName;
    Eigen::Vector4f skydomeEmissiveColor;

    float cloudsEmissiveFactor;
    float sumEmissiveFactor;

    std::string filter2DBName;
    std::string sunBillboard2DBName;
    std::string sunHalo2DBName;
    
    Eigen::Vector4f terrainColor;
    float terrainContrast;

    float contrast;
    float desaturation;
    Eigen::Vector3f balance;

    Eigen::Vector2f carLuminanceRange;
    float carGlobalFactor;
    float carEnvFactor;
    float carAmbiantFactor;
    float carDiffuseFactor;
    float carSpecularFactor;
    float carReflectionFactor;
    float carReflectionLuminosity;
    float carCloudsFactor;
    
    HDRIConfig* pHDRIConfig;
    uint8_t bHDRIEnabled : 1;
};
