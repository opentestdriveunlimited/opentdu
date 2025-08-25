#pragma once

#include "render_file.h"

struct RenderObject {
    uint32_t Flags;
    uint32_t GeometrySectionOffset;
    uint32_t PrimitiveSectionOffset;
    uint32_t NumPrimitives;
    float BoundingSphereOrigin[3];
    float BoundingSphereRadius;
};
static_assert(sizeof(RenderObject) == 0x20, "Must match or else resource parsing will fail");

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

static constexpr uint32_t kObjectMagic = 0x2e4a424f; // .OBJ
