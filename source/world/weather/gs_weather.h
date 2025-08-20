#pragma once

#include "game/gs_base.h"
#include "weather_desc.h"
#include "time_of_day.h"
#include "weather_config.h"
#include "hdri_config.h"

class WeatherConfig;
class WorldListener;

class GSWeather : public GameSystem {
public:
    const char* getName() const override { return "GSWeather"; }
    inline bool isConfigDirty() const { return bDirtyConfig; }

    const WeatherConfig& getActiveWeatherConfig() const { return activeWeatherConfig; }
    const WeatherConfig& getActiveRandomizedWeatherConfig() const { return activeRandomizedWeatherConfig; }

public:
    GSWeather();
    ~GSWeather();

    bool initialize( TestDriveGameInstance* pGameInstance ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;

    void registerListener( WorldListener* pListener );
    void unregisterListener( WorldListener* pListener );
    void updateActiveConfig();

private:
    uint8_t bEnabled : 1;
    uint8_t bFreezeTime : 1;
    uint8_t bUpdateTimerEnabled : 1;
    uint8_t bDirtyConfig : 1;

    float todInterpolationCoeff;
    float timeScale;
    int32_t currentTimeOfDay;

    std::vector<WorldListener*> registeredListeners;
    std::vector<WeatherDesc*> registeredWeatherDescs;
    std::vector<WeatherConfig*> registeredWeatherConfigs;
    std::vector<HDRIConfig*> registeredHDRIConfigs;

    WeatherDesc defaultWeatherDesc;
    std::string defaultWeatherDescName;

    WeatherDesc* pDefaultWeatherDesc;
    WeatherDesc* pCurrentWeatherDesc;

    std::vector<std::string> gameModeWeatherDescs;

    TimeOfDay currentTOD;
    TimeOfDay nextTOD;
    TimeOfDay timeRefreshInterval;

    HDRIConfig activeHDRIConfig;
    WeatherConfig activeWeatherConfig;
    WeatherConfig activeRandomizedWeatherConfig;
};

extern GSWeather* gpWeather;