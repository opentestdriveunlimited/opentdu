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
    // TODO:
};

class RenderDevice {
public:
#if OTDU_WIN32
    inline void bindWindowHandle( HWND handle ) { activeWindow = handle; }
#endif
    inline uint32_t getInternalFrameIndex() const { return frameIndex % PendingFrameCount; }

public:
    RenderDevice();
    ~RenderDevice();

    void initialize();

    GPUTexture*   createTexture( struct Texture* pTexture );
    GPUTexture*   createRenderTarget( const uint32_t width, const uint32_t height, const eViewFormat format, const eAntiAliasingMethod msaaSampleCount );

    GPUBuffer*    createBuffer( const struct GPUBufferDesc* desc );
    GPUShader*    createShader( eShaderType type, const void* pBytecode );

    void* lockBuffer( GPUBuffer* pBuffer, const uint32_t offset, const uint32_t size );
    void unlockBuffer( GPUBuffer* pBuffer );

    void generateMipchain( GPUTexture* pTexture );

    void destroyTexture( GPUTexture* pTexture );
    void destroyBuffer( GPUBuffer* pBuffer );
    void destroyShader( GPUShader* pShader );

    void resize( uint32_t width, uint32_t height );

    GPUBackbuffer* getBackbuffer();
    FormatCapabilities getFormatCapabilities( eViewFormat format );

private:
    // Device extension, extra extensions can be added here
    std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,  // Needed for display on the screen
    };

    VkInstance                         m_instance{};        // The Vulkan instance
    VkPhysicalDevice                   m_physicalDevice{};  // The physical device (GPU)
    VkDevice                           m_device{};          // The logical device (interface to the physical device)
    std::vector<QueueInfo>             m_queues{};          // The queue used to submit command buffers to the GPU
    VkDebugUtilsMessengerEXT           m_callback{ VK_NULL_HANDLE };  // The debug callback
    std::vector<const char*> m_instanceExtensions = { VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME };
    std::vector<const char*> m_instanceLayers = {};  // Add extra layers here
    std::vector<VkExtensionProperties> m_instanceExtensionsAvailable{};
    std::vector<VkExtensionProperties> m_deviceExtensionsAvailable{};
    // Device features, extra features can be added here
    VkPhysicalDeviceFeatures2        m_deviceFeatures;

    VmaAllocator        m_allocator;

    uint32_t frameIndex;
#if OTDU_WIN32
    HWND activeWindow;
#endif

    GPUBackbuffer* pBackbuffer;

    FormatCapabilities formatCaps[eViewFormat::VF_Count];

private:
    QueueInfo getQueue( VkQueueFlagBits flags ) const;
    void getAvailableDeviceExtensions();
};
#endif