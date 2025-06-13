#pragma once

struct TimeOfDay {
    float Hours = 0.0f;
    float Minutes = 0.0f;
    float Seconds = 0.0f;
};

static constexpr TimeOfDay kTimeOfDayMax = { 23.0f, 59.0f, 59.999f };
