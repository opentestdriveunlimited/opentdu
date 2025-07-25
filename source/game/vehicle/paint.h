#pragma once

#include "core/color.h"

enum class ePaintType {
    PT_Default = 0,
    PT_CustomSolid = 1,
    PT_CustomMetal = 2,
    PT_CustomSolidTwoTones = 3,
    PT_CustomMetalTwoTones = 4
};

struct VehiclePaint {
    ePaintType Type;

    int32_t PaintIndex;
    int32_t InteriorIndex;

    ColorRGBA CustomMainColor;
    ColorRGBA CustomSecondColor;
    ColorRGBA CustomTwoToneColor1;
    ColorRGBA CustomTwoToneColor2;

    uint8_t bSaved : 1;
};
