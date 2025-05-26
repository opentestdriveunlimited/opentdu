#pragma once

#include <Eigen/Dense>

struct Material {
    uint16_t OT;
    uint16_t NumParams;
    uint32_t FXFlags;
    uint32_t FXFlags2;
    void* pOTNode;
    uint8_t DepthTest;
    uint8_t DepthWrite;
    uint8_t AlphaSource;
    uint8_t BackfaceCullingMode;
    uint8_t FillMode;
    uint8_t AlphaTest;
    uint8_t MatrixParam;
    uint8_t AlphaRef;
    uint8_t SrcBlend;
    uint8_t DstBlend;
    uint8_t BlendOP;
    uint8_t ColorDisable;
    uint32_t UsageFlags;
    uint8_t StencilEnable;
    uint8_t StencilFail;
    uint8_t StencilZFail;
    uint8_t StencilPass;
    uint8_t StencilFunc;
    uint32_t StencilRef;
    uint32_t StencilMask;
    uint32_t StencilWriteMask;
    uint8_t StencilFailCW;
    uint8_t StencilZFailCW;
    uint8_t StencilPassCW;
    uint8_t StencilFuncCW;
    uint32_t StencilRefCW;
    uint32_t StencilMaskCW;
    uint32_t StenwilWriteMaskCW;
    void* pVertexShaders[8];
    void* pPixelShaders[8];
    Eigen::Vector4f AmbientColor;
    Eigen::Vector4f DiffuseColor;
    Eigen::Vector4f SpecularColor;
    Eigen::Vector4f EmissiveColor;

    Material();
    ~Material();

    void setDefaultMaterial();
};