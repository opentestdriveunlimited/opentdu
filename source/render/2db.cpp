#include "shared.h"
#include "2db.h"

#include "gs_render_helper.h"
#include "gs_render.h"
#include "texture_cache.h"

#include "world/gs_world.h"

static constexpr uint32_t kBitmapMagic = 0x50414d42; // BMAP (BitMAP)
static constexpr uint32_t k2DBMagic    = 0x4244322e; // .2DB (2DBitmap)

Render2DB::Render2DB()
    : RenderFile()
    , pBitmap( nullptr )
{

}

Render2DB::~Render2DB()
{

}

bool Render2DB::hasBeenUploaded() const
{
    OTDU_ASSERT( pBitmap );
    Texture* pTexture = (Texture*)pBitmap;
    return pTexture->bUploaded == 1;
}

Texture* Render2DB::getFirstBitmap() const
{
    OTDU_ASSERT( pBitmap );
    return (Texture*)pBitmap;
}

void Render2DB::create( void* pBuffer, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips, eViewFormat format, uint32_t flags, const char* pName )
{
    uint64_t hashcode = GetIdentifier64bit(pName);

    uint32_t size = Render2DB::CalcTextureSize(width, height, depth, numMips, format, flags);

    uint8_t* puVar3 = (uint8_t*)pBuffer; //((uint32_t)((uint8_t*)pBuffer + 0xf) & 0xfffffff0);
    RenderFile::Header* p2DBHeader = (RenderFile::Header*)puVar3;
    p2DBHeader->VersionMajor = 2;
    p2DBHeader->VersionMinor = 0;
    p2DBHeader->Flags = 0;
    p2DBHeader->Size = size + 0x50;
    p2DBHeader->Hashcode = k2DBMagic;

    RenderFile::Section* pBMapSection  = (RenderFile::Section*)( p2DBHeader + 1 );
    pBMapSection->VersionMajor = 0;
    pBMapSection->VersionMinor = 0;
    pBMapSection->Size = size + 0x40;
    pBMapSection->DataSize = size + 0x40;
    pBMapSection->Type = kBitmapMagic;

    Texture* pTexture = (Texture*)( pBMapSection + 1 );
    pTexture->Flags = flags;
    pTexture->Format = format;
    pTexture->Hashcode = hashcode;
    pTexture->pTexture = nullptr;
    pTexture->pUnused = nullptr;
    pTexture->StreamOffset = 0;
    pTexture->Width = width;
    pTexture->Height = height;
    pTexture->Depth = depth;
    pTexture->NumMipmap = numMips;
    pTexture->Type = ' ';
    pTexture->NumPackedMipmap = 0xcc;
}

bool Render2DB::initialize( void* pBuffer )
{
    if (pBuffer == nullptr) {
        return false;
    }

    parseHeader( pBuffer );
    if (pHeader->Hashcode == k2DBMagic) {
        pBitmap = nullptr;

        if (pHeader->VersionMajor == 2 && pHeader->VersionMinor == 0) {
            bool bVar1 = parseFile();
            if (bVar1) {
                return true;
            }
        }

        reset();
    }

    return false;
}

bool Render2DB::parseSection(RenderFile::Section* pSection)
{  
    if (pSection->Type == kBitmapMagic) {
        pBitmap = pSection;
        
        Texture* pTexture = reinterpret_cast<Texture*>(reinterpret_cast<uint8_t*>(pSection) + sizeof(RenderFile::Section));
        return CreateTexture(pTexture);
    }
    return true;
}

bool Render2DB::CreateTexture(Texture* pTexture)
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
 
        GPUTexture* pGPUTexture = gpRender->getRenderDevice()->createTexture( pTexture );
        if (pGPUTexture == nullptr) {
            OTDU_LOG_ERROR("Failed to create 2DB!\n");
            return false;
        }
        pTexture->pTexture = pGPUTexture;
        if ((pTexture->Flags & 0x400) != 0) {
            PrepareTexture(pTexture);
        }

        return true;
    }
   
    return false;
}

bool Render2DB::PrepareTexture(Texture *pTexture)
{
    if (pTexture->pTexture == nullptr) {
        return false;
    }

    if ((~(pTexture->Flags >> 9) & 1) != 0) {
        UploadTextureToGPU(pTexture);
    }

    if ((pTexture->Flags >> 0xe & 1) != 0) {
        gpRender->getRenderDevice()->generateMipchain(pTexture->pTexture);
    }

    pTexture->bUploaded = true;
    return true;
}

uint32_t Render2DB::CalcRenderTargetSize( Texture* param_1 )
{
    uint32_t texSize = 0;
    if ( param_1->Width != 0 && param_1->Height != 0 && param_1->NumMipmap != 0 ) {
        uint32_t mipWidth = param_1->Width;
        uint32_t mipHeight = param_1->Height;
        for ( int32_t mipIdx = param_1->NumMipmap; mipIdx != 0; mipIdx-- ) {
            uint32_t mipSize = CalcMipSize( mipWidth, mipHeight, param_1->Format );
            texSize += mipSize;

            mipWidth = Max( mipWidth >> 1, 1 );
            mipHeight = Max( mipHeight >> 1, 1 );
        }
    }

    uint32_t arrayLength = 1u;
    if ( ( param_1->Flags >> 0xb & 1 ) != 0 ) {
        OTDU_ASSERT( param_1->Depth >= 1 );
        arrayLength = param_1->Depth;
    }
    if ( ( param_1->Flags >> 6 & 1 ) != 0 ) {
        arrayLength = 6u;
    }

    texSize *= arrayLength;

    return texSize;
}

uint32_t Render2DB::CalcTextureSize( uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips, eViewFormat format, uint32_t flags )
{
    if ( ( flags & 0x200 ) != 0 ) 
    {
        return 0;
    }

    Texture desc;
    desc.Width = width;
    desc.Height = width;
    desc.Depth = depth;
    desc.NumMipmap = numMips;
    desc.Format = format;
    desc.Flags = flags;
    return CalcRenderTargetSize( &desc ) + 15U & 0xfffffff0;
}

int32_t Render2DB::CalcSize( uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips, eViewFormat format, uint32_t flags )
{
    if ( ( flags & 0x200 ) != 0 ) 
    {
        return 0x50;
    }

    return CalcTextureSize( width, height, depth, numMips, format, flags ) + 0x5f & 0xfffffff0;
}

uint32_t Render2DB::CalcPitch( uint32_t width, eViewFormat format )
{
    uint32_t uVar1 = width * 4;

    if ( IsBlockCompressedFormat( format ) ) {
        if ( format == eViewFormat::VF_DXT1 ) {
            return uVar1 >> 3;
        }

        if ( ( ( format != eViewFormat::VF_ATI2 )
               && ( format != eViewFormat::VF_DXT3 ) )
             && ( format != eViewFormat::VF_DXT5 ) ) {
            return uVar1;
        }

        return width & 0x3fffffff;
    }

    if ( ( format != eViewFormat::VF_A16B16G16R16F ) && ( format != eViewFormat::VF_G32R32F ) ) {
        if ( format != eViewFormat::VF_A32B32G32R32F ) {
            return uVar1;
        }

        uVar1 = width * 8;
        return uVar1 * 2;
    }

    if ( format != eViewFormat::VF_R16F ) {
        switch ( format ) {
        case eViewFormat::VF_R5G6B5:
        case eViewFormat::VF_A1R5G5B5:
            // case eViewFormat::VF_D15S1: // AFAIK not defined in their own format enum? (original code uses DX formats)
        case eViewFormat::VF_D16:
        case eViewFormat::VF_L16:
            break;
        default:
            return uVar1;
        case eViewFormat::VF_A16B16G16R16:
        case eViewFormat::VF_Q16W16V16U16:
            return uVar1 * 2;
        case eViewFormat::VF_L8:
            return width & 0x3fffffff;
        }
    }

    return uVar1 >> 1;
}

void* Render2DB::GetTexelsPointer( Texture* pTexture, uint32_t mipIndex )
{
    int32_t uVar1;
    int32_t height;
    int32_t width;

    if ( mipIndex < pTexture->NumMipmap ) {
        height = pTexture->Height;
        width = pTexture->Width;
        void* peVar2 = pTexture + 1;
        if ( mipIndex != 0 ) {
            do {
                uVar1 = Render2DB::CalcMipSize( width, height, pTexture->Format );
                peVar2 = ( uint8_t* )peVar2 + uVar1;
                width = ( int )width / 2;
                if ( width == 0 ) {
                    width = 1;
                }
                height = ( int )height / 2;
                if ( height == 0 ) {
                    height = 1;
                }
                mipIndex = mipIndex - 1;
            } while ( mipIndex != 0 );
        }
        return peVar2;
    }

    return nullptr;
}

int32_t Render2DB::CalcMipSize( int32_t width, int32_t height, eViewFormat format )
{
    return CalcPitch( width, format ) * height;
}

bool Render2DB::UploadTextureToGPU( Texture* param_1 )
{
    uint32_t local_1c = param_1->Width;
    uint32_t local_18 = param_1->Height;
    eViewFormat eVar2 = param_1->Format;
    void* peVar8 = GetTexelsPointer( param_1, 0 );

    uint32_t uVar6 = 0;
    uint32_t local_8 = 0;
    if ( ( param_1->Flags >> 6 & 1 ) != 0 ) {
        uVar6 = CalcRenderTargetSize( param_1 );
        local_8 = uVar6 / 6;
    }
    uVar6 = 0;
    if ( param_1->NumMipmap != 0 ) {
        do {
            uint32_t uVar9 = 0;
            int32_t iVar7 = Render2DB::CalcPitch( local_1c, eVar2 );
            uint32_t mipSize = Render2DB::CalcMipSize( local_1c, local_18, eVar2 );
            if ( IsBlockCompressedFormat( eVar2 ) )
            {
                iVar7 = iVar7 * 4;
                uVar9 = ( int32_t )local_18 >> 2;
            } 
            else
            {
                uVar9 = local_18;
            }

            if ( ( param_1->Flags >> 0xb & 1 ) == 0 ) 
            {
                if ( ( param_1->Flags >> 6 & 1 ) == 0 ) 
                {
                    OTDU_IMPLEMENTATION_SKIPPED( "UploadTexture2D" );
                    // UploadTexture2D( ( IDirect3DTexture9* )pD3DTexture, uVar6, peVar5 );
                } 
                else 
                {
                    OTDU_IMPLEMENTATION_SKIPPED( "UploadTextureCube" );
                    // UploadTextureCube( ( IDirect3DTexture9* )pD3DTexture, uVar6, param_1, iVar8, uVar10, local_8 );
                }
            } 
            else 
            {
                OTDU_IMPLEMENTATION_SKIPPED( "UploadTexture3D" );
                //UploadTexture3D( ( IDirect3DTexture9* )pD3DTexture, uVar6, iVar8, ( uint )param_1, iVar8, ( uint )uVar2
                //                 , ( uint )peVar9 );
            }

            local_1c = local_1c >> 1;
            local_18 = local_18 >> 1;
            uVar6 = uVar6 + 1;
        } while ( uVar6 < param_1->NumMipmap );
    }
    return peVar8;
}

RuntimeRender2DB::RuntimeRender2DB()
    : Render2DB()
    , pBuffer(nullptr)
{
    
}


RuntimeRender2DB::~RuntimeRender2DB()
{
    if (pBuffer != nullptr) {
        TestDrive::Free(pBuffer);
        pBuffer = nullptr;
    }
}

bool RuntimeRender2DB::allocateAndCreate(uint32_t width, 
    uint32_t height, 
    uint32_t depth, 
    uint32_t mipCount, 
    eViewFormat format, 
    uint32_t flags, 
    const char* pLabel
)
{
    uint32_t uVar1 = Render2DB::CalcTextureSize(width, height, depth, mipCount, format, flags);
    if (uVar1 != 0) {
        pBuffer = TestDrive::Alloc( uVar1 );
        create(pBuffer, width, height, depth, mipCount, format, flags, pLabel);
        bool bVar1 = initialize( pBuffer );

        if (bVar1) {
            gpWorld->pushGlobal2DB( this );
            Texture* pTexture = (Texture*)( pBitmap + 1 );
            gTextureCache.insertAndUpload(pTexture, true);
            return true;
        }

        if (pBuffer != nullptr) {
            TestDrive::Free(pBuffer);
            pBuffer = nullptr;
        }
    }

    return false;
}
