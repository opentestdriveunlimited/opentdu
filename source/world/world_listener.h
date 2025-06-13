#pragma once

class WeatherConfig;
struct WeatherDesc;

// Interface to notify time of day change; weather change; etc.
class WorldListener {
public:
    WorldListener();
    ~WorldListener();

    virtual void onTimeOfDayChange(int32_t param_1) {}
    virtual void onWeatherConfigChange(WeatherConfig* param_1 ) {}
    virtual void onWeatherDescChange(WeatherDesc* param_1) {}
};
