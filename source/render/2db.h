#pragma once

#include "render_file.h"
#include "gs_render_helper.h"

class GPUTexture;

struct Texture
{
    uint64_t Hashcode;
    uint16_t Width;
    uint16_t Height;
    uint16_t Depth;
    uint8_t NumMipmap;
    uint8_t NumPackedMipmap;
    uint32_t Flags;
    GPUTexture* pTexture;
    void* pUnused;
    eViewFormat Format;
    uint8_t bUploaded;
    uint8_t Type; // Seems it can only be 3 (DDS)
    uint16_t StreamOffset;
};
static_assert(sizeof(Texture) == 0x30, "Keep this valid (or else streaming will be fucked)");

class Render2DB : public RenderFile {
public:
    Render2DB();
    ~Render2DB();

    virtual bool parseSection(RenderFile::Section* pSection) override;

private:
    RenderFile::Section* pBitmap;

private:
    bool createTexture(Texture* pTexture);
    bool prepareTexture(Texture* pTexture);
    bool uploadTextureToGPU(Texture* pTexture);
};
