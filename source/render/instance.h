#pragma once

#include "lod.h"
#include "render_file.h"
#include "core/color.h"

struct InstanceDef {
    Eigen::Matrix4f Matrix;
    uint32_t Flags;
    RenderFile::Section* pBankMat;
    ColorRGBA Color;
    uint32_t NumLODs;
    std::array<LODDef, 4> LODs;
};

struct Instance {
    uint64_t Hashcode;
    uint32_t Flags;
    RenderFile::Section* pBankMat;
    Eigen::Matrix4f Matrix;
    float BiggestScale;
    float Determinant;
    float DistanceToCurrentLOD;
    uint32_t NumLODs;
    uint32_t CurrentLODIndex;
    Eigen::Matrix4f* pBoneArray;
    ColorRGBA Color;
    float UVTime;
    uint16_t UVAFlags;
    std::array<LOD, 4> LODs;
    uint32_t* pSetupFlags;
    uint32_t SetupMask;
    InstanceDef* pInstanceDef;
};

struct InstanceWithCustomMaterial {
    Instance* pInstance;
    Material* pMaterial;
};
