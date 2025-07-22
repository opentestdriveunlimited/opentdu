#pragma once

#if OTDU_D3D9
// TODO: Split this into multiple cpp!!
#include <d3d9.h>

#include "render/gs_render_helper.h"
#include "render/2dm.h"

struct GPUTexture {
    IDirect3DBaseTexture9* pImpl;
};

struct GPUBuffer {
    union {
        IDirect3DVertexBuffer9* pVertexBuffer;
        IDirect3DIndexBuffer9* pIndexBuffer;
    };
};

struct GPUShader {
    union {
        IDirect3DVertexShader9* pVertexShader;
        IDirect3DPixelShader9* pPixelShader;
    };
};

struct GPUSampler {
    uint32_t AddressModes[3];
    uint32_t MinFilter;
    uint32_t MagFilter;
    uint32_t MipFilter;
};

struct DebugTextRenderer {
    IDirect3DStateBlock9* pStateBlock0 = nullptr;
    IDirect3DStateBlock9* pStateBlock1 = nullptr;
    IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
    IDirect3DBaseTexture9* pTexture = nullptr;
    uint8_t BYTE_0x54 = 0;
};

struct RenderFrameResources {
    IDirect3DSurface9* pDepthStencilRT = nullptr;
    IDirect3DVertexDeclaration9* pTextVertexLayout[2] = { nullptr, nullptr };
    IDirect3DVertexBuffer9* pTextVB = nullptr;
    IDirect3DIndexBuffer9* pTextIB = nullptr;
    void* pText2DMBuffer = nullptr;
    Render2DM text2DM = {};
    Material* pMaterial = nullptr;
    GPUShader* pTextVS = nullptr;
    GPUShader* pTextPS = nullptr;
    void* pVertexShader = nullptr;
    void* pPixelShader = nullptr;

    void* pNextParameterAvailable = nullptr; // TODO: 
    uint32_t Dimensions[4] = { 0, 0, 0, 0 };

    void bindMaterial( Material* param_2 );
    void release();
};

struct VertexStreamCache {
    struct DrawCmd {
        uint16_t PrimitiveCount = 0;
        uint16_t StartIndex = -1;
        uint16_t MinVertexIndex = 0;
        uint16_t NumVertices = 0;
    };

    IDirect3DIndexBuffer9* pIndexBuffer0 = nullptr;
    IDirect3DIndexBuffer9* pIndexBuffer1 = nullptr;
    std::vector<DrawCmd> drawCmds;
    DWORD DWORD_3f0 = 0;

    bool create( IDirect3DDevice9* pDevice );
    void release();
    void FUN_00604e60( IDirect3DDevice9* pDevice );
    int32_t FUN_006059e0();
};

struct GPUDisplayInfos {
    UINT AdapterIndex = 0;
    uint32_t BackbufferWidth = 0;
    uint32_t BackbufferHeight = 0;
    D3DFORMAT BackbufferFormat = D3DFORMAT::D3DFMT_UNKNOWN;
    D3DFORMAT MainDepthBufferFormat = D3DFORMAT::D3DFMT_UNKNOWN;
    uint32_t MSAASampleCount = 1;
    uint32_t RefreshRate = 60;
    uint32_t VSyncMode = 0;
    bool bWindowed = true;
    bool bAllowGPUCapture = false;
    D3DDEVTYPE DeviceType = D3DDEVTYPE::D3DDEVTYPE_HAL;
    UINT BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    eAntiAliasingMethod AntiAliasingMethod = eAntiAliasingMethod::AAM_Disabled;
    eVSyncMode VSync = eVSyncMode::VSM_Immediate;

    std::vector<D3DFORMAT> getCompatBackbufferFormats( IDirect3D9* pDirect3D9, const uint32_t adapterIndex, const bool bWindowed = false );
    void fillPresentInfos( IDirect3D9* param_1, HWND winHandle, D3DPRESENT_PARAMETERS* param_2 );
    void selectAdapter( IDirect3D9* pDirect3D9 );
};

class RenderDevice {
public:
    inline void bindWindowHandle( HWND handle ) { activeWindow = handle; }

public:
    RenderDevice();
    ~RenderDevice();

    void initialize();

    GPUShader* createShader( eShaderType type, const void* pBytecode );
    GPUTexture* createTexture( struct Texture* pTexture );
    GPUBuffer* createBuffer( const struct GPUBufferDesc* desc );

    void* lockBuffer( GPUBuffer* pBuffer, const uint32_t offset, const uint32_t size );
    void unlockBuffer( GPUBuffer* pBuffer );

    void generateMipchain( GPUTexture* pTexture );

    void destroyShader( GPUShader* pShader );
    void destroyTexture( GPUTexture* pTexture );
    void destroyBuffer( GPUBuffer* pBuffer );

    void setGammaRamp( float param_1 );
    bool retrieveAdapterInfos( const uint32_t adapterIndex, struct GPUAdapterDesc* pOutDesc );

    void resize( uint32_t width, uint32_t height );
    int32_t beginFrame();

    void bindTexture( Texture* pTexture, uint32_t index );
    void uploadMipmap( Texture* pTexture, uint32_t mipIndex );

private:
    static constexpr int32_t kMaxBoundTextures = 0x20;
    static constexpr int32_t kMaxSamplerCount = 0x10;
    static constexpr int32_t kNumCachedRasterState = 103;
    static constexpr int32_t kMaxNumCachedVertexStream = 0xc0;

private:
    struct BoundVertexStream {
        IDirect3DVertexBuffer9* pBuffer = nullptr;
        UINT Offset = 0;
        UINT Stride = 0;
    };

    struct BoundRasterState {
        D3DRENDERSTATETYPE State;
        DWORD DefaultValue;
    };

private:
    HWND activeWindow;
    GPUDisplayInfos displayInfos;
    D3DPRESENT_PARAMETERS presentParameters;
    uint32_t frameIndex;
    D3DCAPS9 deviceCaps;
    uint32_t flags;

    uint8_t bSceneEncodingInProgress : 1;

    IDirect3DDevice9* pDevice;
    IDirect3D9* pIDirect3D9;

    // Cached states
    IDirect3DVertexShader9* pBoundVertexShader;
    IDirect3DPixelShader9* pBoundPixelShader;
    IDirect3DIndexBuffer9* pBoundIndexBuffer;

    IDirect3DSurface9* pBoundRenderTargets[kMaxSimultaneousRT];
    IDirect3DBaseTexture9* pBoundTextures[kMaxBoundTextures];

    GPUSampler boundSamplers[kMaxSamplerCount];

    std::unordered_map< D3DRENDERSTATETYPE, DWORD > rasterStates;
    std::unordered_map< D3DRENDERSTATETYPE, DWORD > boundRasterStates;
    DWORD boundRenderStates[kNumCachedRasterState];

    BoundVertexStream boundVertexStreams[kMaxNumCachedVertexStream];
    UINT boundVertexStreamFrequency[kMaxNumCachedVertexStream];
    IDirect3DVertexDeclaration9* pBoundVertexLayout;

    IDirect3DVertexBuffer9* pHmapVertexBuffer;
    IDirect3DVertexDeclaration9* pHmapVertexDecl;
    IDirect3DIndexBuffer9*  pHmapIndexBuffer;

    IDirect3DVertexBuffer9* pLightVertexBuffer;
    IDirect3DVertexDeclaration9* PTR_00fe8a14;
    IDirect3DVertexDeclaration9* PTR_00fe8a18;
    IDirect3DVertexDeclaration9* PTR_00fe8a1c;
    IDirect3DVertexDeclaration9* PTR_00fe8a20;
    uint32_t DAT_00fe8a28;

    RenderFrameResources frameResources;
    DebugTextRenderer textRenderer;
    VertexStreamCache vertexStreamCache; // TODO: Rename me
    
    std::function<void()> postPresentCallback;

private:
    bool createInstance();
    bool createDevice( GPUDisplayInfos* pDisplayInfos, D3DPRESENT_PARAMETERS* pPresentParams );
    bool queryDeviceCaps();

    BYTE FUN_00512e90( uint32_t param_1 );
    bool FUN_00606240( D3DVERTEXELEMENT9* param_1 );

    bool createDebugTextResources();

    bool initializeShaderCache();
    bool resetShaderCache();

    bool resetCachedStates();

    void unbindVertexBuffers();
    void unbindRasterStates();
    void unbindShaders();
    void unbindRenderTargets();
    void unbindTextures();
    void unbindSamplers();

    bool resetPointSize();

    bool flushPendingResourceCreation();
    bool allocateDeviceResources();
    bool freeDeviceResources();

    bool allocateHmapVertexBuffer();
    void releaseHmapVertexBuffer();

    bool createLightBuffer();
    void releaseLightBuffer();

    bool FUN_005f33d0();
    void releaseFrameResources();

    void FUN_005f36e0();
    void FUN_00512350();

    void FUN_005fd350();
    bool FUN_005fd8f0();
};
#endif
