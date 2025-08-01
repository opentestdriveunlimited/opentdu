#pragma once

#include "render_file.h"
#include "gs_render_helper.h"

struct GPUTexture;

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
    inline const uint64_t       getBitmapHashcode() const { OTDU_ASSERT( pBitmap ); return *( uint64_t* )( pBitmap + 1 ); }
    inline RenderFile::Section* getBitmapSection() const { return pBitmap; }

public:
    Render2DB();
    virtual ~Render2DB();

    void create( void* pBuffer, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips, eViewFormat format, uint32_t flags, const char* pName);
    bool initialize( void* pBuffer );

    bool hasBeenUploaded() const;
    Texture* getFirstBitmap() const;
    
    virtual bool parseSection(RenderFile::Section* pSection) override;

    static bool CreateTexture( Texture* pTexture );
    static bool PrepareTexture( Texture* pTexture );
    static bool UploadTextureToGPU( Texture* pTexture );
    
    static uint32_t CalcPitch( uint32_t width, eViewFormat format );
    static int32_t CalcMipSize( int32_t width, int32_t height, eViewFormat format );
    static void* GetTexelsPointer( Texture* pTexture, uint32_t mipIndex );
    static uint32_t CalcRenderTargetSize( Texture* param_1 );

    static uint32_t CalcTextureSize(
        uint32_t width, uint32_t height, uint32_t depth,
        uint32_t numMips,
        eViewFormat format,
        uint32_t flags );

    static int32_t CalcSize(
        uint32_t width, uint32_t height, uint32_t depth,
        uint32_t numMips,
        eViewFormat format,
        uint32_t flags );

protected:
    RenderFile::Section* pBitmap;
};

class RuntimeRender2DB : public Render2DB {
public:
    RuntimeRender2DB();
    ~RuntimeRender2DB();

    bool allocateAndCreate(uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        uint32_t mipCount, 
        eViewFormat format, 
        uint32_t flags, 
        const char* pLabel = "RuntimeRender2DB"
    );

private:
    void*   pBuffer;
};
