#include "shared.h"

#if OTDU_VULKAN
#include "render_device.h"
#include "render/gs_render.h"
#include "render/2db.h"

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
    : frameIndex( 0 )
    , pBackbuffer( nullptr )
{
    memset(formatCaps, 0, sizeof(FormatCapabilities) * eViewFormat::VF_Count);
}

RenderDevice::~RenderDevice()
{

}

QueueInfo RenderDevice::getQueue( VkQueueFlagBits flags ) const
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, nullptr );

    VkQueueFamilyProperties2 defaultProp = {};
    defaultProp.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    std::vector<VkQueueFamilyProperties2> queueFamilies( queueFamilyCount, defaultProp );
    vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, queueFamilies.data() );

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

/*-- Callback function to catch validation errors  -*/
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                     void* )
{
    OTDU_LOG_ERROR("%s\n", callbackData->pMessage );
    /*if ( ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) != 0 )
    {
        OTDU_TRIGGER_BREAKPOINT;
    }*/

    return VK_FALSE;
}

void RenderDevice::getAvailableDeviceExtensions()
{
    uint32_t count{ 0 }; 
    vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &count, nullptr );
    m_deviceExtensionsAvailable.resize( count );
    vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &count, m_deviceExtensionsAvailable.data() );
}

void RenderDevice::initialize()
{
    volkInitialize();

    // This finds the KHR surface extensions needed to display on the right platform
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

    uint32_t count = 0u;
    vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
    m_instanceExtensionsAvailable.resize( count );
    vkEnumerateInstanceExtensionProperties( nullptr, &count, m_instanceExtensionsAvailable.data() );

    VkApplicationInfo applicationInfo = { };
    memset(&applicationInfo, 0, sizeof(VkApplicationInfo));
    applicationInfo.pApplicationName = "OpenTDU";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "OpenTDU";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_2;

    // Add extensions requested by GLFW
    m_instanceExtensions.insert( m_instanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount );
    if ( extensionIsAvailable( VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_instanceExtensionsAvailable ) )
        m_instanceExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );  // Allow debug utils (naming objects)
    if ( extensionIsAvailable( VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, m_instanceExtensionsAvailable ) )
        m_instanceExtensions.push_back( VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME );

    // Setting for the validation layer
    ValidationSettings validationSettings = { };

    // Adding the validation layer
    if ( gVulkanEnableValidationLayer )
    {
        m_instanceLayers.push_back( "VK_LAYER_KHRONOS_validation" );
        validationSettings.validate_core = VK_TRUE;
    }

    VkInstanceCreateInfo instanceCreateInfo;
    memset(&instanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = validationSettings.buildPNextChain();
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = uint32_t( m_instanceLayers.size() );
    instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();
    instanceCreateInfo.enabledExtensionCount = uint32_t( m_instanceExtensions.size() );
    instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensions.data();
#ifdef OTDU_MACOS
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    OTDU_LOG_DEBUG( "Creating Vulkan instance...\n" );
    vkCreateInstance( &instanceCreateInfo, nullptr, &m_instance );
    OTDU_ASSERT( m_instance );

    // Load all Vulkan functions
    volkLoadInstance( m_instance );

    // Add the debug callback
    if ( gVulkanEnableValidationLayer && vkCreateDebugUtilsMessengerEXT )
    {
        VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info = {};
        dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        dbg_messenger_create_info.pfnUserCallback = debugCallback;
        vkCreateDebugUtilsMessengerEXT( m_instance, &dbg_messenger_create_info, nullptr, &m_callback );
    }

    OTDU_LOG_DEBUG( "Enumerating Vulkan devices...\n" );
    size_t chosenDevice = 0;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices( m_instance, &deviceCount, nullptr );
    OTDU_ASSERT( deviceCount != 0 );
    OTDU_LOG_DEBUG( "Found %u devices\n", deviceCount );

    std::vector<VkPhysicalDevice> physicalDevices( deviceCount );
    vkEnumeratePhysicalDevices( m_instance, &deviceCount, physicalDevices.data() );

    VkPhysicalDeviceProperties2 properties2 = { };
    memset(&properties2, 0, sizeof(VkPhysicalDeviceProperties2));
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    for ( size_t i = 0; i < physicalDevices.size(); i++ )
    {
        OTDU_LOG_DEBUG( "Checking device %u...\n", i );
        vkGetPhysicalDeviceProperties2( physicalDevices[i], &properties2 );

        if ( properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
        {
            OTDU_LOG_DEBUG( "Chose device %u\n", i );
            chosenDevice = i;
            break;
        }
    }

    m_physicalDevice = physicalDevices[chosenDevice];
    vkGetPhysicalDeviceProperties2( m_physicalDevice, &properties2 );
    OTDU_LOG_DEBUG( "Selected GPU: %s\n", properties2.properties.deviceName );
    OTDU_LOG_DEBUG( "Driver: %d.%d.%d\n", VK_VERSION_MAJOR( properties2.properties.driverVersion ),
          VK_VERSION_MINOR( properties2.properties.driverVersion ), VK_VERSION_PATCH( properties2.properties.driverVersion ) );
    OTDU_LOG_DEBUG( "Vulkan API: %d.%d.%d\n", VK_VERSION_MAJOR( properties2.properties.apiVersion ),
          VK_VERSION_MINOR( properties2.properties.apiVersion ), VK_VERSION_PATCH( properties2.properties.apiVersion ) );
    const float queuePriority = 1.0F;
    m_queues.clear();
    m_queues.emplace_back( getQueue( VK_QUEUE_GRAPHICS_BIT ) );

    // Request only one queue : graphic
    // User could request more specific queues: compute, transfer
    VkDeviceQueueCreateInfo queueCreateInfo = { };
    memset(&queueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_queues[0].familyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    //// Chaining all features up to Vulkan 1.3
    //pNextChainPushFront( &m_features11, &m_features12 );
    //pNextChainPushFront( &m_features11, &m_features13 );

    ///*--
    // * Check if the device supports the required extensions
    // * Because we cannot request a device with extension it is not supporting
    //-*/
    getAvailableDeviceExtensions();
    if ( extensionIsAvailable( VK_KHR_MAINTENANCE_5_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
       // pNextChainPushFront( &m_features11, &m_maintenance5Features );
        m_deviceExtensions.push_back( VK_KHR_MAINTENANCE_5_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_KHR_MAINTENANCE_6_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
       // pNextChainPushFront( &m_features11, &m_maintenance6Features );
        m_deviceExtensions.push_back( VK_KHR_MAINTENANCE_6_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
        m_deviceExtensions.push_back( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
      //  pNextChainPushFront( &m_features11, &m_dynamicStateFeatures );
        m_deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
      //  pNextChainPushFront( &m_features11, &m_dynamicState2Features );
        m_deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
      //  pNextChainPushFront( &m_features11, &m_dynamicState3Features );
        m_deviceExtensions.push_back( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME );
    }
    if ( extensionIsAvailable( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, m_deviceExtensionsAvailable ) )
    {
       // pNextChainPushFront( &m_features11, &m_swapchainFeatures );
        m_deviceExtensions.push_back( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME );
    }
    //// Requesting all supported features, which will then be activated in the device
    //// By requesting, it turns on all feature that it is supported, but the user could request specific features instead
    //m_deviceFeatures.pNext = &m_features11;
    //vkGetPhysicalDeviceFeatures2( m_physicalDevice, &m_deviceFeatures );

    //ASSERT( m_features13.dynamicRendering, "Dynamic rendering required, update driver!" );
    //ASSERT( m_features13.maintenance4, "Extension VK_KHR_maintenance4 required, update driver!" );  // vkGetDeviceBufferMemoryRequirementsKHR, ...
    //ASSERT( m_maintenance5Features.maintenance5, "Extension VK_KHR_maintenance5 required, update driver!" );  // VkBufferUsageFlags2KHR, ...
    //ASSERT( m_maintenance6Features.maintenance6, "Extension VK_KHR_maintenance6 required, update driver!" );  // vkCmdPushConstants2KHR, vkCmdBindDescriptorSets2KHR

    //// Get information about what the device can do
    //VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    //deviceProperties.pNext = &m_pushDescriptorProperties;
    //vkGetPhysicalDeviceProperties2( m_physicalDevice, &deviceProperties );

    // Create the logical device
    VkDeviceCreateInfo deviceCreateInfo = { };
    memset(&deviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr; // &m_deviceFeatures;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = uint32_t( m_deviceExtensions.size() );
    deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    VkResult op = vkCreateDevice( m_physicalDevice, &deviceCreateInfo, nullptr, &m_device );
    OTDU_ASSERT( op == VK_SUCCESS );

    volkLoadDevice( m_device );  // Load all Vulkan device functions

    // Get the requested queues
    vkGetDeviceQueue( m_device, m_queues[0].familyIndex, m_queues[0].queueIndex, &m_queues[0].queue );

    // Initialize the VMA allocator
    VmaAllocatorCreateInfo infos = { };
    memset(&infos, 0, sizeof(VmaAllocatorCreateInfo));
    infos.physicalDevice = m_physicalDevice;
    infos.device = m_device;
    infos.instance = m_instance;
    infos.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;  // allow querying for the GPU address of a buffer
    infos.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
    infos.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;  // allow using VkBufferUsageFlags2CreateInfoKHR

    VmaVulkanFunctions functions = { };
    memset(&functions, 0, sizeof(VmaVulkanFunctions));
    functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    infos.pVulkanFunctions = &functions;

    vmaCreateAllocator( &infos, &m_allocator );

    //vkGetPhysicalDeviceFormatProperties();
}

GPUTexture* RenderDevice::createTexture( Texture* pTexture )
{
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
        VkResult imageCreationResult = vmaCreateImage( m_allocator, 
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
    imgCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    
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
        VkResult imageCreationResult = vmaCreateImage( m_allocator, 
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
    VkResult opResult = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
    OTDU_ASSERT(opResult == VK_SUCCESS);

    GPUBuffer* pBuffer = new GPUBuffer();
    pBuffer->NativeBuffer = buffer;
    pBuffer->Allocation = allocation;

    return pBuffer;
}

void* RenderDevice::lockBuffer(GPUBuffer *pBuffer, const uint32_t offset, const uint32_t size)
{
    void* pMappedMemory = nullptr;
    VkResult opResult = vmaMapMemory(m_allocator, pBuffer->Allocation, &pMappedMemory);
    OTDU_ASSERT(opResult == VK_SUCCESS);

    return pMappedMemory;
}

void RenderDevice::unlockBuffer(GPUBuffer * pBuffer)
{
    vmaUnmapMemory(m_allocator, pBuffer->Allocation);
}

void RenderDevice::generateMipchain(GPUTexture *pTexture)
{
    OTDU_UNIMPLEMENTED; // TODO
}

void RenderDevice::destroyTexture(GPUTexture *pTexture)
{
    for ( uint32_t i = 0; i < PendingFrameCount; i++ ) {
        vmaDestroyImage( m_allocator,
                         pTexture->Image[i],
                         pTexture->Allocation[i]);
    }
}

void RenderDevice::destroyBuffer(GPUBuffer *pBuffer)
{
    vmaDestroyBuffer(m_allocator, pBuffer->NativeBuffer, pBuffer->Allocation);
}

void RenderDevice::resize( uint32_t width, uint32_t height )
{
    OTDU_IMPLEMENTATION_SKIPPED("RenderDevice::resize");
}

GPUBackbuffer* RenderDevice::getBackbuffer()
{
    return pBackbuffer;
}

FormatCapabilities RenderDevice::getFormatCapabilities( eViewFormat format )
{
    return formatCaps[format];
}

GPUShader* RenderDevice::createShader( eShaderType type, const void* pBytecode )
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

void RenderDevice::destroyShader( GPUShader* pShader )
{
    OTDU_UNIMPLEMENTED;
}
#endif