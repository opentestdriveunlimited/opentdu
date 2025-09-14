#pragma once

#if OTDU_VULKAN
#include <vector>

#define VK_NO_PROTOTYPES 1

#if defined( OTDU_WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif defined( OTDU_MACOS )
#define VK_USE_PLATFORM_MACOS_MVK 1
#endif

#include "volk.h"
#include "vk_mem_alloc.h"
#include "render/gs_render_helper.h"

struct GPUBuffer
{
    VkBuffer        NativeBuffer;
    VmaAllocation   Allocation;
};

/*--
 * A queue is a sequence of commands that are executed in order.
 * The queue is used to submit command buffers to the GPU.
 * The family index is used to identify the queue family (graphic, compute, transfer, ...) .
 * The queue index is used to identify the queue in the family, multiple queues can be in the same family.
-*/
struct QueueInfo {
    uint32_t familyIndex = ~0U;  // Family index of the queue (graphic, compute, transfer, ...)
    uint32_t queueIndex = ~0U;  // Index of the queue in the family
    VkQueue  queue{};            // The queue object
};

static constexpr uint32_t PendingFrameCount = 3;

struct GPUTexture {
    // Allocations for this render target. To get access to the image used for the current frame,
    // retrieve the internal frame index using RenderDevice::getInternalFrameIndex().
    VkImage Image[PendingFrameCount];
    VmaAllocation Allocation[PendingFrameCount];
};

struct GPUShader {
    VkShaderModule ShaderModule;
};

struct GPUPipelineState {
    VkPipeline PipelineState;
};

struct GPUVertexLayout {
    std::vector<VkVertexInputAttributeDescription>  InputAttributes;
    std::vector<VkVertexInputBindingDescription>    InputBindings;
};

class RenderTarget;
class Render2DB;

class RenderDevice {
public:
#if OTDU_WIN32
    inline void bindWindowHandle( HWND handle ) { } // TODO: REMOVE ME
#endif
    inline uint32_t getInternalFrameIndex() const { return frameIndex % PendingFrameCount; }
    inline void setTime(float param_2) { time = param_2; }

    inline bool isInitialized() const { return device != VK_NULL_HANDLE; }
    inline bool isDeviceLost() const { return bDeviceLost; }

public:
    RenderDevice();
    ~RenderDevice();

    void initialize();

    GPUTexture*   createTexture( struct Texture* pTexture );
    GPUTexture*   createRenderTarget( const uint32_t width, const uint32_t height, const eViewFormat format, const eAntiAliasingMethod msaaSampleCount );
    GPUBuffer*    createBuffer( const struct GPUBufferDesc* desc );
    GPUShader*    createShader( eShaderType type, const void* pBytecode, const size_t bytecodeSize );
    GPUPipelineState* createPipelineState( struct Material* pMaterial );
    GPUVertexLayout* createVertexLayout( VertexLayoutAttribute* pAttributes );

    void* lockBuffer( GPUBuffer* pBuffer, const uint32_t offset, const uint32_t size );
    void unlockBuffer( GPUBuffer* pBuffer );

    void generateMipchain( GPUTexture* pTexture );

    void destroyTexture( GPUTexture* pTexture );
    void destroyBuffer( GPUBuffer* pBuffer );
    void destroyShader( GPUShader* pShader );
    void destroyPipelineState( GPUPipelineState* pPipelineState );
    void destroyVertexLayout( GPUVertexLayout* pVertexLayout );

    void resize( uint32_t width, uint32_t height );

    GPUBackbuffer* getBackbuffer();
    FormatCapabilities getFormatCapabilities( eViewFormat format );

    void bindRenderTargetAndSetViewport( RenderTarget* pRenderTarget, const uint32_t index ); // FUN_005f5e10
    void bindRenderTarget( RenderTarget* pRenderTarget, const uint32_t index, const bool bBlitBound2DB ); // FUN_005f16e0
    void blitBound2DBToRT( RenderTarget* pRenderTarget ); // FUN_005fd1d0
    void bindToFramebuffer( RenderTarget* pRenderTarget, const uint32_t index );

    void setViewport( Viewport& vp );
    bool retrieveAdapterInfos( const uint32_t adapterIndex, struct GPUAdapterDesc* pOutDesc );

    // D3D9 Emulation (might want to remove this later and treat this as a regular cbuffer).
    void setFloatConstants(eShaderType stage, float *pFloats, const uint32_t offset, const uint32_t numVectors);

    void bindTexture( Texture* pTexture, const uint32_t index );

    // NOTE: This is done at low level to avoid introducing PSO/pipeline state at
    // high level (and make adoption of old immediate binding style APIs)
    void bindMaterial( Material* pMaterial );

    // Clear ALL attachments (mirrors D3D9 clear behavior). Single RT clear should be implemented in the future.
    void clearFramebuffer(const uint32_t clearColor, const float clearDepth, const uint32_t clearStencil, const bool bClearColor, const bool bClearDepth, const bool bClearStencil);
    void beginRenderPass();

    void bindVertexLayout(GPUVertexLayout* pLayout);
    void bindVertexBuffer(GPUBuffer* pBuffer, const uint32_t index, const uint32_t stride, const uint32_t offset);
    void draw(uint32_t numVertex, uint32_t numInstance, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void drawIndexedPrimitive(
        ePrimitiveType primitiveType,
        int32_t baseVertexIndex,
        uint32_t minVertexIndex,
        uint32_t numVertices,
        uint32_t startIndex,
        uint32_t primitiveCount
    );

    void blit(Render2DB* pSrc, RenderTarget* pDst, const bool bLinearFiltering);

    void resetCachedStates();

    void present();

    RenderTarget* getBoundRenderTargetAtIndex( uint32_t param_1 );
    bool isFramebufferUsingMSAA() const;
    void setMSAAState(bool param_1);

    void bumpInternalFrameIndex();
    void setCullMode( int32_t param_1, int32_t param_2 );

    bool isVertexAttributeFormatSupported(eVertexAttributeFormat format) const;

    void setStreamFrequency( int32_t param_1, int32_t param_3 );
    void setColorWriteChannels(bool bWriteRed, bool bWriteGreen, bool bWriteBlue, bool bWriteAlpha);

private:
    VkInstance          instance;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;
    std::vector<VkExtensionProperties> instanceExtensionsAvailable;
    std::vector<VkExtensionProperties> deviceExtensionsAvailable;

    std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice physicalDevice;
    uint32_t physicalDeviceIndex;
    std::vector<const char*> deviceExtensions;

    VkDevice device;
    std::vector<QueueInfo> queues;

#if OTDU_DEVBUILD
    VkDebugUtilsMessengerEXT           debugCallback;
#endif

    VmaAllocator        allocator;
    VkSurfaceKHR        surface;

    uint32_t frameIndex;

    GPUBackbuffer* pBackbuffer;

    FormatCapabilities formatCaps[eViewFormat::VF_Count];
    uint32_t bufferFormatSupport; // DAT_00fada5c

    // TODO: For now assume single thread cmd submission (should be good enough for a D3D9 game).
    VkCommandBuffer activeCmdBuffer;

    RenderTarget* pBoundRenderTargets[kMaxSimultaneousRT];
    VkFramebufferCreateInfo framebufferInfos;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkSemaphore nextImageSemaphore[PendingFrameCount];

    VkSemaphore frameSubmissionSemaphore[PendingFrameCount];
    VkFence inFlightFence;

    float shaderConstants[2][kNumShaderConstants];

    float time;

    uint8_t bDeviceLost : 1;
    uint8_t bIsMainRTUsingMSAA : 1;

private:
    QueueInfo getQueue( VkQueueFlagBits flags ) const;
    std::vector<VkExtensionProperties> getAvailableDeviceExtensions();

    bool createVulkanInstance();
    bool selectPhysicalDevice();
    bool createDevice();
    void createDebugCallback();
    void createVMAAllocator();
    bool createSwapchain();
};
#endif