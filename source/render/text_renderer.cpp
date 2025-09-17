#include "shared.h"
#include "text_renderer.h"

#include "gs_render_helper.h"
#include "gs_render.h"

TextRenderer gTextRenderer = {}; // DAT_00fc5568
static constexpr const char* kDefaultFontName = "Courier"; // DAT_00eebd90

TextRenderer::TextRenderer()
    : numDrawCommands( 0 )
    , fontName( kDefaultFontName )
    , pGlyphAtlas( nullptr )
    , pGlyphVertexBuffer( nullptr )
    , pStateBlocks{ nullptr, nullptr }
    , flags( 1 )
    , glyphDimension( 0xc )
    , minGlyphDimension( 0 )
    , activeColor( 0xffff0000 )
    , fontScale( 0.0f )
    , glyphAtlasWidth( 0 )
    , glyphAtlasHeight( 0 )
{
    // FUN_009e5a40
}

TextRenderer::~TextRenderer()
{
    // FUN_009eb700
}

bool TextRenderer::initialize()
{
    // FUN_005f33d0
    GPUBufferDesc desc;
    desc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
    desc.Size = 96000;
    desc.bDynamic = true;

    pGlyphVertexBuffer = gpRender->getRenderDevice()->createBuffer(&desc);
    if (pGlyphVertexBuffer == nullptr) {
        return false;
    }

#if 0
    // TODO: D3D9 specific. Make this abstract/high level
    uStack_16d = 1;
    uVar1 = uStack_16d;
    uStack_16d = 1;
    pIStack_168 = ( IDirect3D9* )0x0;
    HVar2 = pDevice->GetDirect3D( &pIStack_168 );
 
    if ( -1 < HVar2 ) {
        pIStack_164 = ( IDirect3DSurface9* )0x0;
        pDevice->GetDeviceCaps( &DStack_130 );
        pDevice->GetDisplayMode( 0, &DStack_160 );
        HVar2 = pDevice->GetRenderTarget( 0, &pIStack_164 );
        uStack_16d = uVar1;
        if ( -1 < HVar2 ) {
            pIStack_164->GetDesc( &DStack_150 );
            uStack_16d = DStack_150.Type;
            D3DRESOURCETYPE type = pIStack_164->GetType();
            if ( type != 0 ) {
                uStack_16d = 0;
            }
            OTDU_D3D9_SAFE_RELEASE( pIStack_164 );
        }
        OTDU_D3D9_SAFE_RELEASE( pIStack_168 );
    }
    uVar3 = 0;
    do {
        pDevice->BeginStateBlock();
        pDevice->SetTexture( 0, textRenderer.pTexture );
        if ( ( textRenderer.BYTE_0x54 & 4 ) == 0 ) {
             pDevice->SetRenderState( D3DRS_ZENABLE, 0 );
        } else {
             pDevice->SetRenderState( D3DRS_ZENABLE, 1 );
        }
        if ( uStack_16d == 0 ) {
             pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
        } else {
             pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
             pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
             pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
        }
         pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 1 );
         pDevice->SetRenderState( D3DRS_ALPHAREF, 8 );
         pDevice->SetRenderState( D3DRS_ALPHAFUNC, 7 );
         pDevice->SetRenderState( D3DRS_FILLMODE, 3 );
         pDevice->SetRenderState( D3DRS_CULLMODE, 3 );
         pDevice->SetRenderState( D3DRS_STENCILENABLE, 0 );
         pDevice->SetRenderState( D3DRS_CLIPPING, 1 );
         pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
         pDevice->SetRenderState( D3DRS_VERTEXBLEND, 0 );
         pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, 0 );
         pDevice->SetRenderState( D3DRS_FOGENABLE, 0 );
         pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xf );
         pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
         pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
         pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
         pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
         pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, 0 );
        if ( uVar3 == 0 ) {
             pDevice->EndStateBlock( &textRenderer.pStateBlock0 );
        } else {
             pDevice->EndStateBlock( &textRenderer.pStateBlock1 );
        }
        uVar3 = uVar3 + 1;
    } while ( uVar3 < 2 );
#endif

    return true;
}

void TextRenderer::pushCommand(int32_t x, int32_t y, const char *pText)
{
    // FUN_005f3870
    uint32_t uVar3 = numDrawCommands;
    if (uVar3 < 0x100) {
        uint32_t textLength = strlen(pText);
        if (textLength < 0x200) {
            DrawCommand& cmd = drawCommands[uVar3];
            cmd.PositionX = x;
            cmd.PositionY = y;
            cmd.Color = activeColor;
            strncpy(cmd.pText, pText, textLength);
            numDrawCommands++;
        }
    }
}

void TextRenderer::setColor(ColorRGBA &param_1)
{
    // FUN_00507020
    activeColor = param_1;
}

void TextRenderer::submitCommands()
{
    // FUN_005f5040
    for (uint32_t i = 0; i < numDrawCommands; i++) {
        const DrawCommand& cmd = drawCommands[i];
        float x = (float)cmd.PositionX;
        if (x < 0.0f) {
            x += 4.2949673e+09f;
        }
        float y = (float)cmd.PositionY;
        if (y < 0.0f) {
            y += 4.2949673e+09f;
        }
        
        submitDrawCommand(x, y, cmd.Color, cmd.pText);
    }

    numDrawCommands = 0;
}

void TextRenderer::submitDrawCommand(const float originX, const float originY, const ColorRGBA& color, const char *pText)
{
    // FUN_005f4160
    OTDU_UNIMPLEMENTED;
}

void DrawDebugText(uint32_t param_1, uint32_t param_2, const char *param_3)
{
    // FUN_00507000
    gTextRenderer.pushCommand(param_1, param_2, param_3);
}
