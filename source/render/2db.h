#pragma once

#include "render_file.h"
#include "gs_render_helper.h"
#include "render_pool.h"

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
OTDU_SIZE_MUST_MATCH(Texture, 0x30);

class Render2DB : public RenderFile {
public:
    inline uint64_t             getBitmapHashcode() const { OTDU_ASSERT( pBitmap ); return *( uint64_t* )( pBitmap + 1 ); }
    inline RenderFile::Section* getBitmapSection() const { return pBitmap; }

public:
    Render2DB();
    virtual ~Render2DB();

    void create( void* pBuffer, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips, eViewFormat format, uint32_t flags, const char* pName);
    bool initialize( void* pBuffer );

    bool hasBeenUploaded() const;
    Texture* getFirstBitmap() const;
    
    virtual void destroy() override;
    virtual bool parseSection(RenderFile::Section* pSection) override;
    virtual void unparseSection( Section* pSection ) override;

    static bool CreateTexture( RenderFile::Section* pBitmapSection );
    static bool PrepareTexture( Texture* pTexture );
    static bool UploadTextureToGPU( Texture* pTexture );
    static bool CreateAndUploadTexture( Texture* pTexture );

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

    static void UploadBitmap( RenderFile::Section* param_1 );

protected:
    RenderFile::Section* pBitmap;

private:
    void unparseBitmapSection(RenderFile::Section* pSection);
};

class RuntimeRender2DB : public Render2DB {
public:
    RuntimeRender2DB();
    ~RuntimeRender2DB();

    void destroy() override;

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

extern RenderPool<RenderFile::Section> gBitmapPool;
