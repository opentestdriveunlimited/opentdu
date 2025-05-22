#include "shared.h"
#include "2db.h"

#include "gs_render_helper.h"
#include "gs_render.h"

static constexpr uint32_t kBitmapMagic = 0x50414d42; // BMAP (BitMAP)

Render2DB::Render2DB()
    : RenderFile()
    , pBitmap( nullptr )
{

}

Render2DB::~Render2DB()
{

}

bool Render2DB::parseSection(RenderFile::Section* pSection)
{  
    if (pSection->Type == kBitmapMagic) {
        pBitmap = pSection;
        
        Texture* pTexture = reinterpret_cast<Texture*>(reinterpret_cast<uint8_t*>(pSection) + sizeof(RenderFile::Section));
        return createTexture(pTexture);
    }
    return true;
}

bool Render2DB::createTexture(Texture* pTexture)
{
    if (pTexture == nullptr) {
        return false;
    }

    if ((pTexture->Type & 3) != 0) {
        uint32_t uVar1 = pTexture->Width;
        if (pTexture->Width < pTexture->Height) {
            uVar1 = pTexture->Height;
        }
        uint32_t iVar4 = 0;
        for (; uVar1 != 0; uVar1 = uVar1 >> 1) {
            iVar4 = iVar4 + 1;
        }
        if (iVar4 < pTexture->NumMipmap) {
            pTexture->NumMipmap = static_cast<uint8_t>(iVar4);
        }

        GPUTextureDesc textureDesc( 
            pTexture->Width, 
            pTexture->Height, 
            pTexture->Depth,
            pTexture->NumMipmap, 
            pTexture->Format, 
            pTexture->Flags
        );

        GPUTexture* pGPUTexture = gpRender->getRenderDevice()->createTexture( &textureDesc );
        if (pGPUTexture == nullptr) {
            OTDU_LOG_ERROR("Failed to create 2DB!\n");
            return false;
        }
        pTexture->pTexture = pGPUTexture;
        if ((pTexture->Flags & 0x400) != 0) {
            prepareTexture(pTexture);
        }

        return true;
    }
   
    return false;
}

bool Render2DB::prepareTexture(Texture *pTexture)
{
    if (pTexture->pTexture == nullptr) {
        return false;
    }

    if ((~(pTexture->Flags >> 9) & 1) != 0) {
        uploadTextureToGPU(pTexture);
    }

    if ((pTexture->Flags >> 0xe & 1) != 0) {
        gpRender->getRenderDevice()->generateMipchain(pTexture->pTexture);
    }

    pTexture->bUploaded = true;
    return true;
}

bool Render2DB::uploadTextureToGPU(Texture *pTexture)
{
    OTDU_UNIMPLEMENTED; // TODO:
    return false;
}
