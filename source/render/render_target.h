#pragma once

#include "gs_render_helper.h"

struct GPUTexture;

struct RenderTarget {
    uint32_t Width = 0u;
    uint32_t Height = 0u;
    eViewFormat Format = eViewFormat::VF_Invalid;
    eAntiAliasingMethod AA = eAntiAliasingMethod::AAM_Disabled;
    GPUTexture* pUnderlyingTexture = nullptr;
    uint32_t Flags = 0u;
    eShaderType BoundStage = eShaderType::ST_Invalid;
};
