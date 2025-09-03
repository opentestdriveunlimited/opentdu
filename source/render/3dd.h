#pragma once

#include "render_file.h"

struct RenderObject {
    uint32_t Flags;
    x86Pointer_t GeometrySection;
    x86Pointer_t PrimitiveSection;
    uint32_t NumPrimitives;
    Eigen::Vector4f BoundingSphere; // Origin (xyz) + Radius (w)
};
OTDU_SIZE_MUST_MATCH(RenderObject, 0x20);

struct Heightmap : public RenderObject {
    uint32_t NumTiles;
    uint32_t NumTileX;
    uint32_t NumTileZ;
    float    TileSize;
};
OTDU_SIZE_MUST_MATCH(Heightmap, 0x30);

class Render3DD : public RenderFile {
public:
    Render3DD();
    ~Render3DD();

    virtual bool parseSection(RenderFile::Section* pSection) override;

private:
    RenderFile::Section* pMatBankArray;
    RenderFile::Section* pObjectArray;
    RenderFile::Section* pInstanceArray;
    RenderFile::Section* pHiearchyArray;
};

static constexpr uint32_t kObjectMagic      = 0x2e4a424f; // .OBJ
static constexpr uint32_t kHeightmapMagic   = 0x50414d48; // HMAP
