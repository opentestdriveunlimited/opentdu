#pragma once

#include "lod.h"
#include "render_file.h"
#include "core/color.h"
#include "render/setup_node.h"

class Camera;
struct Material;

static constexpr uint32_t kMaxNumLOD = 4;

struct InstanceDef {
    Eigen::Matrix4f Matrix;
    uint32_t Flags;
    RenderFile::Section* pBankMat;
    ColorRGBA Color;
    uint32_t NumLODs;
    std::array<LODDef, 4> LODs;
};

class Instance {
public:
    inline uint32_t& getFlagsWrite() { return flags; }
    inline uint32_t getFlags() const { return flags; }
    inline uint32_t getNumLODs() const { return numLODs; }
    inline uint32_t getActiveLODIndex() const { return currentLODIndex; }
    inline LOD& getLOD(const uint32_t param_1) { return lods[param_1]; }
    inline RenderFile::Section* getMaterialBank() const { return pBankMat; }
    inline const Eigen::Matrix4f& getModelMatrix() const { return matrix; }
    inline float getBoundingScale() const { return biggestScale; }
    inline LOD* getLODs() { return lods.data(); }
    inline const SetupGraph* getSetup() const { return &setupGraph; }
    inline Eigen::Matrix4f* getBones() const { return pBoneArray; }
    inline const ColorRGBA& getColor() const { return color; }

public:
    Instance();
    ~Instance();

    void calculateLOD(Camera* param_2);

private:
    uint64_t hashcode;
    uint32_t flags;
    RenderFile::Section* pBankMat;
    Eigen::Matrix4f matrix;
    float biggestScale;
    float determinant;
    float distanceToCurrentLOD;
    uint32_t numLODs;
    uint32_t currentLODIndex;
    Eigen::Matrix4f* pBoneArray;
    ColorRGBA color;
    float uvTime;
    uint16_t uvaFlags;
    std::array<LOD, kMaxNumLOD> lods;
    SetupGraph setupGraph;
    InstanceDef* pInstanceDef;
};

struct InstanceWithCustomMaterial {
    Instance* pInstance;
    Material* pMaterial;
};
