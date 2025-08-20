#include "shared.h"

#if OTDU_VULKAN
#include "render_device.h"
#include "render/2db.h"
#include "render/render_target.h"
#include "system/gs_system.h"
#include "core/arg_parser.h"

#define GLFW_INCLUDE_VULKAN 1
#include "GLFW/glfw3.h"

#define VK_NO_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#define VOLK_IMPLEMENTATION 1
#include "volk.h"

#if defined(OTDU_MACOS)
#include <MoltenVK/mvk_vulkan.h>
#endif

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION 1
#include "vk_mem_alloc.h"

static bool gVulkanEnableValidationLayer = false;
static CmdLineArg CmdLineArgVulkanValidationLayer( "vk_enable_validation_layer", []( const char* pArg ) {
    gVulkanEnableValidationLayer = true;
} );

static constexpr const VkFormat kDepth24Stencil8 = VK_FORMAT_D24_UNORM_S8_UINT; // VF_D24S8
static constexpr const VkFormat kDepth24Stencil8Float = VK_FORMAT_D32_SFLOAT;   // VF_D24S8F
static constexpr const VkFormat kDepth16 = VK_FORMAT_D16_UNORM;                 // VF_D16

static constexpr const VkFormat kViewFormatLUT[eViewFormat::VF_Count] = {
    VK_FORMAT_R8G8B8_SINT,                  // VF_R8G8B8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_R8G8B8A8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_X8R8G8B8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_A8R8G8B8
    VK_FORMAT_R5G6B5_UNORM_PACK16,          // VF_R5G6B5
    VK_FORMAT_A1R5G5B5_UNORM_PACK16,        // VF_A1R5G5B5
    VK_FORMAT_R8_UNORM,                     // VF_A8
    VK_FORMAT_UNDEFINED,                    // VF_A8R3G3B2
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,     // VF_A2R10G10B10
    VK_FORMAT_R16G16_UNORM,                 // VF_G16R16
    VK_FORMAT_R16G16B16A16_UNORM,           // VF_A16B16G16R16
    VK_FORMAT_R8_UNORM,                     // VF_L8
    VK_FORMAT_R16_UNORM,                    // VF_L16
    VK_FORMAT_R8G8_SNORM,                   // VF_V8U8
    VK_FORMAT_R8G8B8A8_SNORM,               // VF_Q8W8V8U8
    VK_FORMAT_R16G16_SNORM,                 // VF_V16U16
    VK_FORMAT_R16G16B16A16_SNORM,           // VF_Q16W16V16U16
    VK_FORMAT_R16_SFLOAT,                   // VF_R16F
    VK_FORMAT_R16G16_SFLOAT,                // VF_G16FR16F
    VK_FORMAT_R16G16B16A16_SFLOAT,          // VF_A16FB16FG16FR16F
    VK_FORMAT_R32_SFLOAT,                   // VF_R32F
    VK_FORMAT_R32G32_SFLOAT,                // VF_G32FR32F
    VK_FORMAT_R32G32B32A32_SFLOAT,          // VF_A32FB32FG32FR32F
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,         // VF_DXT1
    VK_FORMAT_BC2_UNORM_BLOCK,              // VF_DXT2
    VK_FORMAT_BC3_UNORM_BLOCK,              // VF_DXT3
    VK_FORMAT_BC4_UNORM_BLOCK,              // VF_DXT4
    VK_FORMAT_BC5_UNORM_BLOCK,              // VF_DXT5
    
    // TODO: Need to figure out what those are (sRGB formats? linear formats?)
    VK_FORMAT_R8G8B8_SINT,                  // VF_R8G8B8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_R8G8B8A8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_X8R8G8B8
    VK_FORMAT_B8G8R8A8_UNORM,               // VF_A8R8G8B8
    VK_FORMAT_R5G6B5_UNORM_PACK16,          // VF_R5G6B5
    VK_FORMAT_A1R5G5B5_UNORM_PACK16,        // VF_A1R5G5B5
    VK_FORMAT_R8_UNORM,                     // VF_A8
    VK_FORMAT_UNDEFINED,                    // VF_A8R3G3B2
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,     // VF_A2R10G10B10
    VK_FORMAT_R16G16_UNORM,                 // VF_G16R16
    VK_FORMAT_R16G16B16A16_UNORM,           // VF_A16B16G16R16
    VK_FORMAT_R8_UNORM,                     // VF_L8
    VK_FORMAT_R16_UNORM,                    // VF_L16
    VK_FORMAT_R8G8_SNORM,                   // VF_V8U8
    VK_FORMAT_R8G8B8A8_SNORM,               // VF_Q8W8V8U8
    VK_FORMAT_R16G16_SNORM,                 // VF_V16U16
    VK_FORMAT_R16G16B16A16_SNORM,           // VF_Q16W16V16U16
    VK_FORMAT_R16_SFLOAT,                   // VF_R16F
    VK_FORMAT_R16G16_SFLOAT,                // VF_G16FR16F
    VK_FORMAT_R16G16B16A16_SFLOAT,          // VF_A16FB16FG16FR16F
    VK_FORMAT_R32_SFLOAT,                   // VF_R32F
    VK_FORMAT_R32G32_SFLOAT,                // VF_G32FR32F
    VK_FORMAT_R32G32B32A32_SFLOAT,          // VF_A32FB32FG32FR32F
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,         // VF_DXT1
    VK_FORMAT_BC2_UNORM_BLOCK,              // VF_DXT2
    VK_FORMAT_BC3_UNORM_BLOCK,              // VF_DXT3
    VK_FORMAT_BC4_UNORM_BLOCK,              // VF_DXT4
    VK_FORMAT_BC5_UNORM_BLOCK,              // VF_DXT5

    VK_FORMAT_UNDEFINED, // VF_ATI2
    VK_FORMAT_UNDEFINED, // VF_Count
    VK_FORMAT_UNDEFINED, // VF_Invalid
};

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                     void* )
{
    OTDU_LOG_ERROR("%s\n", callbackData->pMessage );
    if ( ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) != 0 )
    {
        OTDU_TRIGGER_BREAKPOINT;
    }

    return VK_FALSE;
}

// Validation settings: to fine tune what is checked
struct ValidationSettings {
    VkBool32 fine_grained_locking{ VK_TRUE };
    VkBool32 validate_core{ VK_TRUE };
    VkBool32 check_image_layout{ VK_TRUE };
    VkBool32 check_command_buffer{ VK_TRUE };
    VkBool32 check_object_in_use{ VK_TRUE };
    VkBool32 check_query{ VK_TRUE };
    VkBool32 check_shaders{ VK_TRUE };
    VkBool32 check_shaders_caching{ VK_TRUE };
    VkBool32 unique_handles{ VK_TRUE };
    VkBool32 object_lifetime{ VK_TRUE };
    VkBool32 stateless_param{ VK_TRUE };
    std::vector<const char*> debug_action{ "VK_DBG_LAYER_ACTION_LOG_MSG" };  // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
    std::vector<const char*> report_flags{ "error" };

    VkBaseInStructure* buildPNextChain()
    {
        layerSettings = std::vector<VkLayerSettingEXT>{
            {layerName, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &fine_grained_locking},
            {layerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &validate_core},
            {layerName, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_image_layout},
            {layerName, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_command_buffer},
            {layerName, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_object_in_use},
            {layerName, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_query},
            {layerName, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders},
            {layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders_caching},
            {layerName, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &unique_handles},
            {layerName, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &object_lifetime},
            {layerName, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &stateless_param},
            {layerName, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t( debug_action.size() ), debug_action.data()},
            {layerName, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t( report_flags.size() ), report_flags.data()},

        };
        layerSettingsCreateInfo.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
        layerSettingsCreateInfo.settingCount = uint32_t( layerSettings.size() );
        layerSettingsCreateInfo.pSettings = layerSettings.data();

        return reinterpret_cast< VkBaseInStructure* >( &layerSettingsCreateInfo );
    }

    static constexpr const char* layerName{ "VK_LAYER_KHRONOS_validation" };
    std::vector<VkLayerSettingEXT> layerSettings{};
    VkLayerSettingsCreateInfoEXT   layerSettingsCreateInfo{};
};

// Work in conjunction with the above
bool extensionIsAvailable( const std::string& name, const std::vector<VkExtensionProperties>& extensions )
{
    for ( auto& ext : extensions )
    {
        if ( name == ext.extensionName )
            return true;
    }
    return false;
}

RenderDevice::RenderDevice()
    : instance( VK_NULL_HANDLE )
    , physicalDevice( VK_NULL_HANDLE )
    , physicalDeviceIndex( 0u )
    , device( VK_NULL_HANDLE )
#if OTDU_DEVBUILD
    , debugCallback( VK_NULL_HANDLE )
#endif
    , surface( nullptr )
    , frameIndex( 0 )
    , pBackbuffer( nullptr )
    , activeCmdBuffer( VK_NULL_HANDLE )
    , framebufferInfos{}
    , time(0.0f)
{
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    memset(pBoundRenderTargets, 0, sizeof(RenderTarget*) * kMaxSimultaneousRT);
    memset(formatCaps, 0, sizeof(FormatCapabilities) * eViewFormat::VF_Count);
    memset(shaderConstants, 0, sizeof(float) * 2 * kNumShaderConstants);
}

RenderDevice::~RenderDevice()
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);

    physicalDevice = VK_NULL_HANDLE;
    physicalDevices.clear();
    deviceExtensions.clear();

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

QueueInfo RenderDevice::getQueue( VkQueueFlagBits flags ) const
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2( physicalDevice, &queueFamilyCount, nullptr );

    VkQueueFamilyProperties2 defaultProp = {};
    defaultProp.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    std::vector<VkQueueFamilyProperties2> queueFamilies( queueFamilyCount, defaultProp );
    vkGetPhysicalDeviceQueueFamilyProperties2( physicalDevice, &queueFamilyCount, queueFamilies.data() );

    QueueInfo queueInfo;
    for ( uint32_t i = 0; i < queueFamilies.size(); i++ )
    {
        if ( queueFamilies[i].queueFamilyProperties.queueFlags & flags )
        {
            queueInfo.familyIndex = i;
            queueInfo.queueIndex = 0;  // A second graphic queue could be index 1 (need logic to find the right one)
            // m_queueInfo.queue = After creating the logical device
            break;
        }
    }
    return queueInfo;
}

std::vector<VkExtensionProperties> RenderDevice::getAvailableDeviceExtensions()
{
    std::vector<VkExtensionProperties> availableDeviceExtensions;
    uint32_t count = 0u;
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &count, nullptr );
    availableDeviceExtensions.resize( count );
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &count, availableDeviceExtensions.data() );
    return availableDeviceExtensions;
}

void RenderDevice::initialize()
{
    OTDU_ASSERT_FATAL( volkInitialize() == VK_SUCCESS );
    OTDU_ASSERT_FATAL( createVulkanInstance() );
    createDebugCallback();
    OTDU_ASSERT_FATAL( selectPhysicalDevice() );
    OTDU_ASSERT_FATAL( createDevice() );
    OTDU_ASSERT_FATAL( createSwapchain() );
    createVMAAllocator();

    // Retrieve format capabilities
    for ( uint32_t iVar7 = 0; iVar7 < eViewFormat::VF_Count; iVar7++ ) {
        VkImageFormatProperties properties = { 0 };
        VkResult opResult = vkGetPhysicalDeviceImageFormatProperties(
            physicalDevice,
            kViewFormatLUT[iVar7],
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            0,
            &properties
        );

        if ( opResult == VK_SUCCESS ) {
            formatCaps[iVar7].bSupported = true;
            formatCaps[iVar7].bSupportBlending = true;
            formatCaps[iVar7].bSupportMSAAx2 = properties.sampleCounts >= 2;
            formatCaps[iVar7].bSupportMSAAx4 = properties.sampleCounts >= 4;
        } else {
            OTDU_LOG_WARN("Format %u is unsupported by this device!\n", iVar7);
        }
    }
}

GPUTexture* RenderDevice::createTexture( Texture* pTexture )
{
    uint16_t uVar1 = pTexture->Width;
    if ( pTexture->Width < pTexture->Height ) {
        uVar1 = pTexture->Height;
    }
    uint8_t iVar4 = 0;
    for ( ; uVar1 != 0; uVar1 = uVar1 >> 1 ) {
        iVar4 = iVar4 + 1;
    }
    if ( iVar4 < pTexture->NumMipmap ) {
        pTexture->NumMipmap = iVar4;
    }
    
    const bool bVolumeTex = (pTexture->Flags >> 0xb & 1) == 0;

    VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imgCreateInfo.imageType = bVolumeTex ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    imgCreateInfo.extent.width = pTexture->Width;
    imgCreateInfo.extent.height = pTexture->Height;
    imgCreateInfo.extent.depth = pTexture->Depth;
    imgCreateInfo.mipLevels = pTexture->NumMipmap;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.format = kViewFormatLUT[pTexture->Format];
    imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT; // Textures are assumed to be read only
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    GPUTexture* renderTarget = new GPUTexture();

    for ( uint32_t i = 0; i < PendingFrameCount; i++ ) {
        VkResult imageCreationResult = vmaCreateImage( allocator, 
                                                       &imgCreateInfo, 
                                                       &allocCreateInfo, 
                                                       &renderTarget->Image[i], 
                                                       &renderTarget->Allocation[i], 
                                                       nullptr );
        OTDU_ASSERT( imageCreationResult == VK_SUCCESS );
    }

    return renderTarget;
}

GPUTexture* RenderDevice::createRenderTarget( const uint32_t width, const uint32_t height, const eViewFormat format, const eAntiAliasingMethod msaaSampleCount )
{
    VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D; // Only 2D RT are used AFAIK
    imgCreateInfo.extent.width = width;
    imgCreateInfo.extent.height = height;
    imgCreateInfo.extent.depth = 1;
    imgCreateInfo.mipLevels = 1;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.format = kViewFormatLUT[format];
    imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: Could assume DSV layout bit if the gmae never samples it
    imgCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (IsDepthStencilFormat(format)) {
        imgCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    } else {
        imgCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    switch (msaaSampleCount) {
    case eAntiAliasingMethod::AAM_MSAA_X2:
        imgCreateInfo.samples = VK_SAMPLE_COUNT_2_BIT;
        break;
    case eAntiAliasingMethod::AAM_MSAA_X4:
        imgCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
        break;
    default:
        imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        break;
    }

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    GPUTexture* renderTarget = new GPUTexture();

    for ( uint32_t i = 0; i < PendingFrameCount; i++ ) {
        VkResult imageCreationResult = vmaCreateImage( allocator, 
                                                       &imgCreateInfo, 
                                                       &allocCreateInfo, 
                                                       &renderTarget->Image[i], 
                                                       &renderTarget->Allocation[i], 
                                                       nullptr );
        OTDU_ASSERT( imageCreationResult == VK_SUCCESS );
    }

    return renderTarget;
}

GPUBuffer *RenderDevice::createBuffer( const struct GPUBufferDesc* desc )
{
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = desc->Size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // TODO: This is currently extremely simplistic (since TDU1 is a D3D9 game).
    // This could be extended to add new render passes/modernize the game rendering pipeline.
    switch( desc->BufferType ) {
    case GPUBufferDesc::Type::BT_VertexBuffer:
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case GPUBufferDesc::Type::BT_IndexBuffer:
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    default:
        OTDU_UNIMPLEMENTED;
        break;
    };

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkBuffer buffer;
    VmaAllocation allocation;
    VkResult opResult = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
    OTDU_ASSERT(opResult == VK_SUCCESS);

    GPUBuffer* pBuffer = new GPUBuffer();
    pBuffer->NativeBuffer = buffer;
    pBuffer->Allocation = allocation;

    return pBuffer;
}

void* RenderDevice::lockBuffer(GPUBuffer *pBuffer, const uint32_t offset, const uint32_t size)
{
    void* pMappedMemory = nullptr;
    VkResult opResult = vmaMapMemory(allocator, pBuffer->Allocation, &pMappedMemory);
    OTDU_ASSERT(opResult == VK_SUCCESS);

    return pMappedMemory;
}

void RenderDevice::unlockBuffer(GPUBuffer * pBuffer)
{
    vmaUnmapMemory(allocator, pBuffer->Allocation);
}

void RenderDevice::generateMipchain(GPUTexture *pTexture)
{
    OTDU_IMPLEMENTATION_SKIPPED("RenderDevice::generateMipchain");
}

void RenderDevice::destroyTexture(GPUTexture *pTexture)
{
    for ( uint32_t i = 0; i < PendingFrameCount; i++ ) {
        vmaDestroyImage( allocator,
                         pTexture->Image[i],
                         pTexture->Allocation[i]);
    }
}

void RenderDevice::destroyBuffer(GPUBuffer *pBuffer)
{
    vmaDestroyBuffer(allocator, pBuffer->NativeBuffer, pBuffer->Allocation);
}

void RenderDevice::resize( uint32_t width, uint32_t height )
{
    OTDU_IMPLEMENTATION_SKIPPED("RenderDevice::resize");
}

void RenderDevice::destroyPipelineState(GPUPipelineState *pPipelineState)
{
    OTDU_IMPLEMENTATION_SKIPPED("RenderDevice::destroyPipelineState");
}

GPUBackbuffer *RenderDevice::getBackbuffer()
{
    return pBackbuffer;
}

FormatCapabilities RenderDevice::getFormatCapabilities( eViewFormat format )
{
    return formatCaps[format];
}

void RenderDevice::bindRenderTargetAndSetViewport(RenderTarget *pRenderTarget, const uint32_t index)
{
    bindRenderTarget(pRenderTarget, index, false);

    VkViewport viewport = { 0.0f };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = pRenderTarget->getWidth();
    viewport.height = pRenderTarget->getHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport( activeCmdBuffer, 0, 1, &viewport );
}

void RenderDevice::bindRenderTarget(RenderTarget* pRenderTarget, const uint32_t index, const bool bBlitBound2DB)
{
    if (index < kMaxSimultaneousRT && pRenderTarget != pBoundRenderTargets[index]) {
        if (pRenderTarget != nullptr) {
            bindToFramebuffer(pRenderTarget, index);
            if (bBlitBound2DB) {
                blitBound2DBToRT(pRenderTarget);
            }
        }

        pBoundRenderTargets[index] = pRenderTarget;
    }
}

void RenderDevice::blitBound2DBToRT(RenderTarget *pRenderTarget)
{
    // NOTE: This is slightly different from the original code (as the game
    // used to manually copy the bitmap to the RT with a fullscreen quad).
    // Since Vulkan offers an explicit cmd to blit textures we'll use that instead
    // to save time and reduce risk.
    Render2DB* pBound2DB = pRenderTarget->getBound2DB();
    if (pBound2DB == nullptr) {
        return;
    }
    
    blit(pBound2DB, pRenderTarget, false);
}

void RenderDevice::bindToFramebuffer(RenderTarget *pRenderTarget, const uint32_t index)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::setViewport(Viewport &vp)
{
    VkViewport viewport;
    viewport.x = vp.X;
    viewport.y = vp.Y;
    viewport.width = vp.Width;
    viewport.height = vp.Height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport( activeCmdBuffer, 0, 1, &viewport );
}

void RenderDevice::setFloatConstants(eShaderType stage, float *pFloats, uint32_t numFloat)
{
    OTDU_ASSERT( stage == eShaderType::ST_Vertex || stage == eShaderType::ST_Pixel );

    uint32_t stageIndex = (stage == eShaderType::ST_Vertex) ? 0 : 1;
    memcpy(shaderConstants[stageIndex], pFloats, numFloat * sizeof(float));
}

void RenderDevice::bindTexture( Texture* pTexture, const uint32_t index )
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::bindMaterial( Material* pMaterial )
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::clearFramebuffer(const bool bClearColor, const bool bClearDepth, const bool bClearStencil)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::beginRenderPass()
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::bindVertexBuffer(GPUBuffer * pBuffer, const uint32_t index, const uint32_t stride)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::draw(uint32_t numVertex, uint32_t numInstance, uint32_t firstVertex, uint32_t firstInstance)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::blit(Render2DB *pBound2DB, RenderTarget *pDst, const bool bLinearFiltering)
{
    Texture* pSrc = pBound2DB->getFirstBitmap();
    GPUTexture* pTextureRT = pDst->getTextureColor();

    VkImage srcImage = pSrc->pTexture->Image[0];
    VkImage dstImage = pTextureRT->Image[frameIndex % PendingFrameCount];

    VkImageBlit region = {};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffsets[0] = { 0, 0, 0 };
    region.srcOffsets[1] = { pSrc->Width, pSrc->Height, pSrc->Depth };

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstOffsets[0] = { 0, 0, 0 };
    region.dstOffsets[1].x = pDst->getWidth();
    region.dstOffsets[1].y = pDst->getHeight();
    region.dstOffsets[1].z = 1;

    // TODO: Check if we could potentially use copy instead (to save some GPU time)
    vkCmdBlitImage( activeCmdBuffer, 
                    srcImage,
                    VK_IMAGE_LAYOUT_GENERAL,
                    dstImage,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    1,
                    &region,
                    VK_FILTER_NEAREST
    );
}

GPUShader* RenderDevice::createShader( eShaderType type, const void* pBytecode, const size_t bytecodeSize )
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.codeSize = bytecodeSize;
    createInfo.pCode = (const uint32_t*)pBytecode;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return nullptr;
    }
    
    GPUShader* pShader = new GPUShader();
    pShader->ShaderModule = shaderModule;
    return pShader;
}

GPUPipelineState* RenderDevice::createPipelineState( struct Material* pMaterial )
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

    OTDU_UNIMPLEMENTED;
    // VkPipelineShaderStageCreateInfo infoVS {};
    // infoVS.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // infoVS.pNext = nullptr;
    // infoVS.stage = VK_SHADER_STAGE_VERTEX_BIT;
    // infoVS.module = shaderModule;
    // infoVS.pName = "main";

    return nullptr;
}

void RenderDevice::destroyShader( GPUShader* pShader )
{
    OTDU_UNIMPLEMENTED;
}

bool RenderDevice::retrieveAdapterInfos( const uint32_t adapterIndex, struct GPUAdapterDesc* pOutDesc )
{
    if (adapterIndex >= physicalDevices.size()) {
        OTDU_LOG_WARN("Failed to retrieve adapter %u infos (index out of bounds)\n", adapterIndex);
        return false;
    }

    VkPhysicalDeviceProperties2 properties2 = {};
    memset(&properties2, 0, sizeof(VkPhysicalDeviceProperties2));
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2( physicalDevice, &properties2 );

    pOutDesc->AdapterName = properties2.properties.deviceName;
    pOutDesc->DeviceDesc = properties2.properties.deviceName;
    pOutDesc->DeviceID = properties2.properties.deviceID;
    pOutDesc->VendorId = properties2.properties.vendorID;
    pOutDesc->Revision = properties2.properties.driverVersion;
    pOutDesc->SubSysId = 0;

    return true;
}

bool RenderDevice::createVulkanInstance()
{
    // This finds the KHR surface extensions needed to display on the right platform
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

    uint32_t count = 0u;
    vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
    instanceExtensionsAvailable.resize( count );
    vkEnumerateInstanceExtensionProperties( nullptr, &count, instanceExtensionsAvailable.data() );

    VkApplicationInfo applicationInfo = { };
    memset(&applicationInfo, 0, sizeof(VkApplicationInfo));
    applicationInfo.pApplicationName = "OpenTDU";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "OpenTDU";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_2;

    // Add extensions requested by GLFW
    instanceExtensions.insert( instanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount );
    if ( extensionIsAvailable( VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instanceExtensionsAvailable ) ) {
        instanceExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );  // Allow debug utils (naming objects)
    }

    if ( extensionIsAvailable( VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, instanceExtensionsAvailable ) ) {
        instanceExtensions.push_back( VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME );
    }

    // Setting for the validation layer
    ValidationSettings validationSettings = { };

    // Adding the validation layer
    if ( gVulkanEnableValidationLayer ) {
        instanceLayers.push_back( "VK_LAYER_KHRONOS_validation" );
        validationSettings.validate_core = VK_TRUE;
    }

    VkInstanceCreateInfo instanceCreateInfo;
    memset(&instanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = validationSettings.buildPNextChain();
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = uint32_t( instanceLayers.size() );
    instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceCreateInfo.enabledExtensionCount = uint32_t( instanceExtensions.size() );
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
#ifdef OTDU_MACOS
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    OTDU_LOG_DEBUG( "Creating Vulkan instance...\n" );

    VkResult opResult = vkCreateInstance( &instanceCreateInfo, nullptr, &instance );

    if (opResult != VK_SUCCESS || instance == nullptr) {
        return false;
    }
    
    // Load all Vulkan functions
    volkLoadInstance( instance );

    return true;
}

bool RenderDevice::selectPhysicalDevice()
{    
    OTDU_LOG_DEBUG( "Enumerating Vulkan devices...\n" );
    physicalDeviceIndex = std::numeric_limits<uint32_t>::max();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );
    OTDU_ASSERT( deviceCount != 0 );
    OTDU_LOG_DEBUG( "Found %u devices\n", deviceCount );

    physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices( instance, &deviceCount, physicalDevices.data() );

    VkPhysicalDeviceProperties2 properties2 = { };
    memset(&properties2, 0, sizeof(VkPhysicalDeviceProperties2));
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    for ( uint32_t i = 0; i < physicalDevices.size(); i++ ) {
        OTDU_LOG_DEBUG( "Checking device %u...\n", i );
        vkGetPhysicalDeviceProperties2( physicalDevices[i], &properties2 );

        if ( properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
            OTDU_LOG_DEBUG( "Chose device %u\n", i );
            physicalDeviceIndex = i;
            break;
        }
    }

    if (physicalDeviceIndex == std::numeric_limits<uint32_t>::max()) {
        if (!physicalDevices.empty()) {
            OTDU_LOG_WARN( "Could not find dedicated GPU on this system; will use the first one available...\n");
            physicalDeviceIndex = 0;
        } else {
            OTDU_LOG_ERROR( "Could not find compatible GPU!\n" );
            return false;
        }
    }

    physicalDevice = physicalDevices[physicalDeviceIndex];
    vkGetPhysicalDeviceProperties2( physicalDevice, &properties2 );

    OTDU_LOG_DEBUG( "Selected GPU: %s\n", properties2.properties.deviceName );
    OTDU_LOG_DEBUG( "Driver: %d.%d.%d\n", VK_VERSION_MAJOR( properties2.properties.driverVersion ),
          VK_VERSION_MINOR( properties2.properties.driverVersion ), VK_VERSION_PATCH( properties2.properties.driverVersion ) );
    OTDU_LOG_DEBUG( "Vulkan API: %d.%d.%d\n", VK_VERSION_MAJOR( properties2.properties.apiVersion ),
          VK_VERSION_MINOR( properties2.properties.apiVersion ), VK_VERSION_PATCH( properties2.properties.apiVersion ) );
    
    return true;
}

bool RenderDevice::createDevice()
{
    const float queuePriority = 1.0F;
    queues.clear();
    queues.emplace_back( getQueue( VK_QUEUE_GRAPHICS_BIT ) );

    // Request only one queue : graphic
    // User could request more specific queues: compute, transfer
    VkDeviceQueueCreateInfo queueCreateInfo = { };
    memset(&queueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queues[0].familyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    std::vector<VkExtensionProperties> availableDeviceExtensions = getAvailableDeviceExtensions();
    if ( extensionIsAvailable( VK_KHR_MAINTENANCE_5_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_KHR_MAINTENANCE_5_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_KHR_MAINTENANCE_6_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_KHR_MAINTENANCE_6_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, availableDeviceExtensions ) ) {
        deviceExtensions.push_back( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME );
    }

    // Create the logical device
    VkDeviceCreateInfo deviceCreateInfo = { };
    memset(&deviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr; // &deviceFeatures;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = uint32_t( deviceExtensions.size() );
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    VkResult op = vkCreateDevice( physicalDevice, &deviceCreateInfo, nullptr, &device );
    if (op != VK_SUCCESS) {
        return false;
    }

    volkLoadDevice( device );  // Load all Vulkan device functions

    // Get the requested queues
    vkGetDeviceQueue( device, queues[0].familyIndex, queues[0].queueIndex, &queues[0].queue );

    return true;
}

void RenderDevice::createDebugCallback()
{
#if OTDU_DEVBUILD
    // Add the debug callback
    if ( gVulkanEnableValidationLayer && vkCreateDebugUtilsMessengerEXT ) {
        VkDebugUtilsMessengerCreateInfoEXT createInfos = {};
        createInfos.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfos.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfos.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        createInfos.pfnUserCallback = VulkanDebugCallback;
        vkCreateDebugUtilsMessengerEXT( instance, &createInfos, nullptr, &debugCallback );
    }
#endif
}

void RenderDevice::createVMAAllocator()
{
    // Initialize the VMA allocator
    VmaAllocatorCreateInfo infos = { };
    memset(&infos, 0, sizeof(VmaAllocatorCreateInfo));
    infos.physicalDevice = physicalDevice;
    infos.device = device;
    infos.instance = instance;
    infos.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;  // allow querying for the GPU address of a buffer
    infos.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
    infos.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;  // allow using VkBufferUsageFlags2CreateInfoKHR

    VmaVulkanFunctions functions = { };
    memset(&functions, 0, sizeof(VmaVulkanFunctions));
    functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    infos.pVulkanFunctions = &functions;

    vmaCreateAllocator( &infos, &allocator );
}

static constexpr VkFormat kBackbufferFormats[4] = {
    VK_FORMAT_A2R10G10B10_UNORM_PACK32, // D3DFMT_A2B10G10R10,
    VK_FORMAT_B8G8R8A8_SRGB, // D3DFMT_X8R8G8B8,
    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R5G6B5_UNORM_PACK16
};

bool RenderDevice::createSwapchain()
{
    // Create window surface
    VkResult surfCreation = glfwCreateWindowSurface(instance, gpSystem->getWindow(), nullptr, &surface );
    if ( surfCreation != VK_SUCCESS ) {
        return false;
    }

    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount = 0u;
    std::vector<VkSurfaceFormatKHR> formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount == 0) {
        return false;
    }
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    uint32_t presentModeCount = 0u;
    std::vector<VkPresentModeKHR> presentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount == 0) {
        return false;
    }
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    // Select backbuffer format
    uint32_t pickedFormatIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t j = 0; j < formats.size(); j++) {
        VkSurfaceFormatKHR& surfaceFormat = formats[j];

        bool bMatchingFormat = false;
        for (uint32_t i = 0; i < 4; i++) {
            if (surfaceFormat.format == kBackbufferFormats[i] 
             && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                bMatchingFormat = true;
                break;
            }
        }

        if (bMatchingFormat) {
            pickedFormatIndex = j;
            break;
        }
    }
    OTDU_ASSERT_FATAL( pickedFormatIndex != std::numeric_limits<uint32_t>::max() );

    // Select Present Mode
    bool bSupportMailboxPresent = false;
    for (VkPresentModeKHR presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            bSupportMailboxPresent = true;
            break;
        }
    }

    VkSurfaceFormatKHR swapchainFormat = formats[pickedFormatIndex];
    VkPresentModeKHR presentMode = (bSupportMailboxPresent) ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;    
    VkExtent2D swapchainDimension = capabilities.currentExtent;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapchainFormat.format;
    createInfo.imageColorSpace = swapchainFormat.colorSpace;
    createInfo.imageExtent = swapchainDimension;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult opResult = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
    if (opResult != VK_SUCCESS) {
        return false;
    }
    
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    return true;
}
#endif