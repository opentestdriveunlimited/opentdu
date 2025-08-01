#include "shared.h"
#include "render_target.h"

#include "2db.h"
#include "gs_render_helper.h"
#include "gs_render.h"

static RenderTargetPool gRenderTargetPool = {};
static RenderTarget* gBackbuffer = nullptr; // DAT_00faf454

RenderTarget::RenderTarget()
    : pNextRT(nullptr)
    , width( 0.0f )
    , height( 0.0f )
    , widthNoPadding( -1.0f )
    , heightNoPadding( -1.0f )
    , format( VF_Invalid )
    , antiAliasing( eAntiAliasingMethod::AAM_Disabled )
    , p2DB( nullptr )
    , flags( 0 )
    , flagsUnknown( 0 )
    , boundStage( eShaderType::ST_Invalid )
    , pTexture( nullptr )
    , pDepthBuffer( nullptr )
    , bOwnTexture( false )
    , bOwnZBuffer( false )
{
    
}

RenderTarget::~RenderTarget()
{

}

void RenderTarget::destroy( RenderDevice* pDevice )
{
    pDevice->destroyTexture(pTexture);
    pDevice->destroyTexture(pDepthBuffer);
}

void RenderTarget::bind2DB( Render2DB *param_1 )
{
    if (param_1 == nullptr || !param_1->hasBeenUploaded()) {
        p2DB = param_1;
    }
}

bool RenderTarget::create()
{
    if ((flagsUnknown >> 3 & 1) != 0) {
      return createFromBackbuffer();
    }

    if (pParentRT != nullptr) {
        return createWithCopy(pParentRT, flags);
    }

    if (p2DB != nullptr) {
        return createFrom2DB(p2DB, flags);
    }

    return create(width, height, format, antiAliasing, flags);
}

bool RenderTarget::create(float texWidth, float texHeight, const eViewFormat viewFormat, const eAntiAliasingMethod aaMethod, const uint32_t texFlags)
{
  // FUN_005fd3e0
  bool bVar1 = false;
  bool bVar2 = false;
  bool bVar3 = false;
  uint32_t uVar4 = 0;
  float originalWidth = -1.0f;
  float originalHeight = -1.0f;
  
  FormatCapabilities caps = gpRender->getRenderDevice()->getFormatCapabilities(viewFormat);
  if (!caps.bSupported) {
    OTDU_LOG_ERROR("Could not create render target: view format %u is not render target capable!\n", viewFormat);
    return false;
  }

  bool bSupportSampleCount = true;
  switch (aaMethod) {
  case eAntiAliasingMethod::AAM_MSAA_X2:
    bSupportSampleCount = caps.bSupportMSAAx2;
    break;
  case eAntiAliasingMethod::AAM_MSAA_X4:
    bSupportSampleCount = caps.bSupportMSAAx4;
    break;
  default:
    break;
  };
  
  if (!bSupportSampleCount) {
    OTDU_LOG_ERROR("Could not create render target: view format %u does not support the requested number of sample %u!\n", viewFormat, aaMethod);
    return false;
  }

  // Calls CheckDepthStencilMatch() against the default depthstencil format.
  // I don't think this is still a thing with modern gfx APIs so we can skip this call
  // bVar3 = FUN_00512f60(viewFormat);
  // if (!bVar3) {
  //   viewFormat = VF_X8R8G8B8; // Fail safe to create a RT compatible with the main zbuffer
  // }

  if ((texFlags >> 4 & 1) == 0) {
    originalWidth = -1.0;
    originalHeight = -1.0;
  } else {
    uVar4 = gBackbuffer->getWidth();
    originalWidth = texWidth;
    originalHeight = texHeight;
    texWidth = texWidth * (float)uVar4;
    uVar4 = gBackbuffer->getHeight();
    texHeight = texHeight * (float)uVar4;
  }

  uint32_t roundedWidth = ( uint32_t )texWidth;
  uint32_t roundedHeight = ( uint32_t )texHeight;

  pTexture = gpRender->getRenderDevice()->createRenderTarget(
      roundedWidth,
      roundedHeight,
      viewFormat,
      aaMethod
  );

  if (pTexture == nullptr) {
      return false;
  }
  bOwnTexture = true;

  if ( ( texFlags & 1 ) != 0 ) {
    if ( ( texFlags >> 5 & 1 ) != 0 ) {
        pDepthBuffer = gpRender->getRenderDevice()->createRenderTarget( roundedWidth, roundedHeight, gDepthStencilFormat, aaMethod);
        if (pDepthBuffer == nullptr) {
            return false;
        }
        bOwnZBuffer = true;
    } else {
        pDepthBuffer = gpMainDepthBuffer;
        bOwnZBuffer = false;
    }
  }
  antiAliasing = aaMethod;
  widthNoPadding = originalWidth;
  uVar4 = 0;
  width = texWidth;
  height = texHeight;
  heightNoPadding = originalHeight;
  format = viewFormat;
  flags = texFlags;
  if ((texFlags & 1) != 0) {
    uVar4 = 4;
  }
  if ((texFlags >> 5 & 1) != 0) {
    uVar4 = uVar4 | 0x20;
  }
  if ((texFlags >> 4 & 1) != 0) {
    uVar4 = uVar4 | 0x10;
  }
  flagsUnknown = uVar4;
  return true;
}
   
bool RenderTarget::createFrom2DB( Render2DB* param_2, uint32_t uStack_4 )
{
    // FUN_005fd690
    const Texture* pUnderlyingTexture = param_2->getFirstBitmap();
    
    width = pUnderlyingTexture->Width;
    height = pUnderlyingTexture->Height;
    format = pUnderlyingTexture->Format;
    antiAliasing = eAntiAliasingMethod::AAM_Disabled;
    bind2DB( param_2 );
   
    pTexture = gpRender->getRenderDevice()->createRenderTarget(
        ( uint32_t )width,
        ( uint32_t )height,
        format,
        antiAliasing
    );

    if (pTexture == nullptr) {
        return false;
    }

    bool bUseMainDepthBuffer = (uStack_4 >> 1) & 1;
    bool bNoDepthBuffer = (uStack_4 >> 2) & 1;
    if (!bNoDepthBuffer) {
        if (!bUseMainDepthBuffer) {
            pDepthBuffer = gpRender->getRenderDevice()->createRenderTarget( ( uint32_t )width, ( uint32_t )height, gDepthStencilFormat, antiAliasing);
            if (pDepthBuffer == nullptr) {
                return false;
            }
            bOwnZBuffer = true;
        } else {
            pDepthBuffer = gpMainDepthBuffer;
            bOwnZBuffer = false;
        }
    }

    uint32_t uVar4 = 0;
    flags = uStack_4;
    if ((uStack_4 & 1) != 0) {
        uVar4 = 4;
    }
    if (bUseMainDepthBuffer) {
        uVar4 = uVar4 | 0x20;
    }
    if (bNoDepthBuffer) {
        uVar4 = uVar4 | 0x10;
    }
    flagsUnknown = uVar4;
    return true;
}

bool RenderTarget::createWithCopy( RenderTarget* param_2, const uint32_t param_3 )
{
    width = param_2->width;
    height = param_2->height;
    format = param_2->format;
    antiAliasing = param_2->antiAliasing;
    uint32_t uVar2 = 0;
    flags = param_3;
    if ((param_3 & 1) != 0) {
        uVar2 = 4;
    }
    if ((param_3 >> 5 & 1) != 0) {
        uVar2 = uVar2 | 0x20;
    }
    if ((param_3 >> 4 & 1) != 0) {
        uVar2 = uVar2 | 0x10;
    }
    flagsUnknown = uVar2;
    pTexture = param_2->pTexture;
    pDepthBuffer = param_2->pDepthBuffer;
    pParentRT = param_2;
    return true;
}

bool RenderTarget::createFromBackbuffer()
{
    GPUBackbuffer* pBackbuffer = gpRender->getRenderDevice()->getBackbuffer();
    if (pBackbuffer != nullptr) {
        width = ( float )pBackbuffer->Width;
        height = ( float )pBackbuffer->Height;

        format = pBackbuffer->Format;
        antiAliasing = eAntiAliasingMethod::AAM_Disabled;

        pTexture = pBackbuffer->pTexture;
        pDepthBuffer = nullptr;

        p2DB = nullptr;
        flags = 0;
        flagsUnknown = 8;

        pParentRT = nullptr;

        bOwnTexture = true;
        bOwnZBuffer = false;

        // SetDebugTag("BackBuffer");
        return true;
    }
 
    return false;
}

eViewFormat RenderTarget::getFormat() const
{
    return format;
}

void RenderTarget::setFlagsUnknown(const uint32_t param_1)
{
    flagsUnknown = param_1;
}

uint32_t RenderTarget::getWidth()
{
    return ( uint32_t )width;
}

uint32_t RenderTarget::getHeight()
{
    return ( uint32_t )height;
}

RenderTarget* RenderTarget::GetBackBuffer()
{
    return gBackbuffer;
}

RenderTargetPool::RenderTargetPool()
    : pListHead( nullptr )
    , numPooledRT( 0 )
{

}

RenderTargetPool::~RenderTargetPool()
{

}

RenderTarget* RenderTargetPool::allocateUninitialized()
{
    return createAndAddToPool();
}

RenderTarget* RenderTargetPool::allocate(const float width, const float height, const eViewFormat format, const uint32_t flags)
{
    RenderTarget* peVar2 = createAndAddToPool();

    bool bVar1 = peVar2->create(width, height, format, eAntiAliasingMethod::AAM_Disabled, flags); 
    if (!bVar1) {
        peVar2->destroy(gpRender->getRenderDevice());
        removeFromPool(peVar2);
        TestDrive::Free(peVar2);
        return nullptr;
    }
    return peVar2;
}

RenderTarget* RenderTargetPool::allocateFromOriginal(RenderTarget* param_1, const uint32_t flags)
{
    RenderTarget* peVar2 = createAndAddToPool();

    bool bVar1 = peVar2->createWithCopy(param_1, flags); 
    if (!bVar1) {
        peVar2->destroy(gpRender->getRenderDevice());
        removeFromPool(peVar2);
        TestDrive::Free(peVar2);
        return nullptr;
    }
    return peVar2;
}

void RenderTargetPool::removeFromPool( RenderTarget* param_1 )
{
    if (pListHead != nullptr) {
        RenderTarget* peVar1 = pListHead;
        RenderTarget* peVar2 = nullptr;
        RenderTarget* peVar3 = nullptr;
        while (peVar2 = pListHead, peVar2 != param_1) {
            peVar1 = peVar2->pNextRT;
            peVar3 = peVar2;
            if (peVar2->pNextRT == nullptr) {
                return;
            }
        }
        if (peVar2 != nullptr) {
            if (peVar3 != nullptr) {
                numPooledRT--;
                peVar3->pNextRT = param_1->pNextRT;
                param_1->pNextRT = nullptr;
                return;
            }
            pListHead = param_1->pNextRT;
            numPooledRT--;
            param_1->pNextRT = nullptr;
        }
    }
}

RenderTarget* RenderTargetPool::createAndAddToPool()
{
    RenderTarget* peVar2 = new (TestDrive::Alloc(sizeof(RenderTarget))) RenderTarget();
    peVar2->pNextRT = pListHead;
    pListHead = peVar2;
    numPooledRT++;

    return peVar2;
}

RenderTarget* CreateRenderTarget(const uint32_t width, const uint32_t height, const eViewFormat format, const eAntiAliasingMethod aaMethod, const uint32_t flags )
{
    // FUN_0050afa0
    if ( ( ( ( ( flags >> 4 ) & 1 )  == 0 ) && ( 0.0f < ( float )width && 0.0f < ( float )height ) )
      || ( width != 0 && height != 0 ) ) {
        return gRenderTargetPool.allocate(( float )width, ( float )height, format, flags);
    }

    return nullptr;
}

RenderTarget* CopyRenderTarget(RenderTarget* param_1, const uint32_t param_2)
{
    if (param_1 == nullptr) {
        return nullptr;
    }

    return gRenderTargetPool.allocateFromOriginal(param_1, param_2);
}

bool CreateBackbufferRenderTarget()
{
    gBackbuffer = gRenderTargetPool.allocateUninitialized();

    if (gBackbuffer == nullptr) {
        return false;
    }

    gBackbuffer->setFlagsUnknown(8);
    return true;
}

void ReleaseBackbufferRenderTarget()
{
    if (gBackbuffer != nullptr) {
        gBackbuffer->destroy(gpRender->getRenderDevice());
        gRenderTargetPool.removeFromPool(gBackbuffer);
        TestDrive::Free(gBackbuffer);
    }
}

RenderTarget* FUN_0050aff0(RenderTarget *param_1, float param_2, float param_3, eViewFormat param_4, uint32_t param_5, uint32_t flags)
{
    // TODO: Not sure what's the purpose of param_1 in this context (could be sanity check/conditional alloc?)
    if ((~(int8_t)(flags >> 4) & 1) == 0) {
        if (param_2 <= 0.0) {
            return nullptr;
        }
        if (param_3 <= 0.0) {
            return nullptr;
        }
    } else {
        if (param_2 == 0.0) {
            return nullptr;
        }
        if (param_3 == 0.0) {
            return nullptr;
        }
    }

    if (param_1 == nullptr) {
        return nullptr;
    }

    return CreateRenderTarget(( uint32_t )param_2, ( uint32_t )param_3, param_4, eAntiAliasingMethod::AAM_Disabled, param_5);
}
