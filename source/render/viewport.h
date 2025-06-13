#pragma once

#include "core/color.h"

struct Viewport {
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 1.0f;
    float Height = 1.0f;
    ColorRGBA ClearColor = gColorBlackNoAlpha;
    uint32_t Flags = 0x7;
};
