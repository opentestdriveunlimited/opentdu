#include "shared.h"

#if OTDU_OPENGL
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "render_device.h"
#include "render/gs_render_helper.h"

static constexpr const uint32_t kViewFormatPixelFormatLUT[eViewFormat::VF_Count] = {
    GL_RGB_INTEGER, // VF_R8G8B8
    GL_RGBA, // VF_R8G8B8A8
    GL_RGBA, // VF_X8R8G8B8
    GL_RGBA, // VF_A8R8G8B8
    GL_RGB, // VF_R5G6B5
    0x0, // VF_A1R5G5B5
    GL_RED, // VF_A8
    0x0, // VF_A8R3G3B2
    0x0, // VF_A2R10G10B10
    GL_RG, // VF_G16R16
    GL_RGBA, // VF_A16B16G16R16
    GL_RED,                     // VF_L8
    GL_RED,                    // VF_L16
    GL_RG,              // VF_V8U8
    GL_RGBA,            // VF_Q8W8V8U8
    GL_RG,             // VF_V16U16
    GL_RGBA,           // VF_Q16W16V16U16
    GL_RED,                   // VF_R16F
    GL_RG,                  // VF_G16FR16F
    GL_RGBA,                // VF_A16FB16FG16FR16F
    GL_RED,                   // VF_R32F
    GL_RG,                  // VF_G32FR32F
    GL_RGBA,                // VF_A32FB32FG32FR32F
    GL_RGB,   // VF_DXT1
    GL_RGBA,  // VF_DXT2
    GL_RGBA,  // VF_DXT3
    GL_RED,   // VF_DXT4
    GL_RGBA,  // VF_DXT5

    GL_RGB_INTEGER, // VF_R8G8B8
    GL_RGBA, // VF_R8G8B8A8
    GL_RGBA, // VF_X8R8G8B8
    GL_RGBA, // VF_A8R8G8B8
    GL_RGB, // VF_R5G6B5
    0x0, // VF_A1R5G5B5
    GL_RED, // VF_A8
    0x0, // VF_A8R3G3B2
    0x0, // VF_A2R10G10B10
    GL_RG, // VF_G16R16
    GL_RGBA, // VF_A16B16G16R16
    GL_RED,                     // VF_L8
    GL_RED,                    // VF_L16
    GL_RG,              // VF_V8U8
    GL_RGBA,            // VF_Q8W8V8U8
    GL_RG,             // VF_V16U16
    GL_RGBA,           // VF_Q16W16V16U16
    GL_RED,                   // VF_R16F
    GL_RG,                  // VF_G16FR16F
    GL_RGBA,                // VF_A16FB16FG16FR16F
    GL_RED,                   // VF_R32F
    GL_RG,                  // VF_G32FR32F
    GL_RGBA,                // VF_A32FB32FG32FR32F
    GL_RGB,   // VF_DXT1
    GL_RGBA,  // VF_DXT2
    GL_RGBA,  // VF_DXT3
    GL_RED,           // VF_DXT4
    GL_RGBA,  // VF_DXT5

    0,
    0,
    0
};
 
static constexpr const uint32_t kViewFormatTexelFormatLUT[eViewFormat::VF_Count] = {
    GL_BYTE, // VF_R8G8B8
    GL_UNSIGNED_BYTE, // VF_R8G8B8A8
    GL_UNSIGNED_BYTE, // VF_X8R8G8B8
    GL_UNSIGNED_BYTE, // VF_A8R8G8B8
    GL_UNSIGNED_SHORT_5_6_5_REV, // VF_R5G6B5
    0x0, // VF_A1R5G5B5
    GL_UNSIGNED_BYTE, // VF_A8
    0x0, // VF_A8R3G3B2
    0x0, // VF_A2R10G10B10
    GL_UNSIGNED_SHORT, // VF_G16R16
    GL_UNSIGNED_SHORT, // VF_A16B16G16R16
    GL_UNSIGNED_BYTE,                     // VF_L8
    GL_UNSIGNED_BYTE,                    // VF_L16
    GL_UNSIGNED_SHORT,              // VF_V8U8
    GL_BYTE,            // VF_Q8W8V8U8
    GL_SHORT,             // VF_V16U16
    GL_SHORT,           // VF_Q16W16V16U16
    GL_HALF_FLOAT,                   // VF_R16F
    GL_HALF_FLOAT,                  // VF_G16FR16F
    GL_FLOAT,                // VF_A16FB16FG16FR16F
    GL_FLOAT,                   // VF_R32F
    GL_FLOAT,                  // VF_G32FR32F
    GL_FLOAT,                // VF_A32FB32FG32FR32F
    GL_BYTE,   // VF_DXT1
    GL_BYTE,  // VF_DXT2
    GL_BYTE,  // VF_DXT3
    GL_BYTE,   // VF_DXT4
    GL_BYTE,  // VF_DXT5
 GL_BYTE, // VF_R8G8B8
    GL_UNSIGNED_BYTE, // VF_R8G8B8A8
    GL_UNSIGNED_BYTE, // VF_X8R8G8B8
    GL_UNSIGNED_BYTE, // VF_A8R8G8B8
    GL_UNSIGNED_SHORT_5_6_5_REV, // VF_R5G6B5
    0x0, // VF_A1R5G5B5
    GL_UNSIGNED_BYTE, // VF_A8
    0x0, // VF_A8R3G3B2
    0x0, // VF_A2R10G10B10
    GL_UNSIGNED_SHORT, // VF_G16R16
    GL_UNSIGNED_SHORT, // VF_A16B16G16R16
    GL_UNSIGNED_BYTE,                     // VF_L8
    GL_UNSIGNED_BYTE,                    // VF_L16
    GL_UNSIGNED_SHORT,              // VF_V8U8
    GL_BYTE,            // VF_Q8W8V8U8
    GL_SHORT,             // VF_V16U16
    GL_SHORT,           // VF_Q16W16V16U16
    GL_HALF_FLOAT,                   // VF_R16F
    GL_HALF_FLOAT,                  // VF_G16FR16F
    GL_FLOAT,                // VF_A16FB16FG16FR16F
    GL_FLOAT,                   // VF_R32F
    GL_FLOAT,                  // VF_G32FR32F
    GL_FLOAT,                // VF_A32FB32FG32FR32F
    GL_BYTE,   // VF_DXT1
    GL_BYTE,  // VF_DXT2
    GL_BYTE,  // VF_DXT3
    GL_BYTE,   // VF_DXT4
    GL_BYTE,  // VF_DXT5

    0,
    0,
    0
};
 
static constexpr const uint32_t kViewFormatLUT[eViewFormat::VF_Count] = {
    GL_RGB8I,                  // VF_R8G8B8
    GL_RGBA8,                  // VF_R8G8B8A8
    GL_RGBA8,                  // VF_X8R8G8B8
    GL_RGBA8,                  // VF_A8R8G8B8
    GL_RGB565,                 // VF_R5G6B5
    0x0,                       // VF_A1R5G5B5
    GL_R8,                     // VF_A8
    0x0,                       // VF_A8R3G3B2
    0x0,                       // VF_A2R10G10B10
    GL_RG16,                   // VF_G16R16
    GL_RGBA16,                 // VF_A16B16G16R16
    GL_R8,                     // VF_L8
    GL_R16,                    // VF_L16
    GL_RG8_SNORM,              // VF_V8U8
    GL_RGBA8_SNORM,            // VF_Q8W8V8U8
    GL_RG16_SNORM,             // VF_V16U16
    GL_RGBA16_SNORM,           // VF_Q16W16V16U16
    GL_R16F,                   // VF_R16F
    GL_RG16F,                  // VF_G16FR16F
    GL_RGBA16F,                // VF_A16FB16FG16FR16F
    GL_R32F,                   // VF_R32F
    GL_RG32F,                  // VF_G32FR32F
    GL_RGBA32F,                // VF_A32FB32FG32FR32F
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,   // VF_DXT1
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,  // VF_DXT2
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,  // VF_DXT3
    GL_COMPRESSED_RED_RGTC1,           // VF_DXT4
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,  // VF_DXT5

    // TODO: Need to figure out what those are (sRGB formats? linear formats?)
    GL_RGB8I,                  // VF_R8G8B8
    GL_RGBA8,                  // VF_R8G8B8A8
    GL_RGBA8,                  // VF_X8R8G8B8
    GL_RGBA8,                  // VF_A8R8G8B8
    GL_RGB565,                 // VF_R5G6B5
    0x0,                       // VF_A1R5G5B5
    GL_R8,                     // VF_A8
    0x0,                       // VF_A8R3G3B2
    0x0,                       // VF_A2R10G10B10
    GL_RG16,                   // VF_G16R16
    GL_RGBA16,                 // VF_A16B16G16R16
    GL_R8,                     // VF_L8
    GL_R16,                    // VF_L16
    GL_RG8_SNORM,              // VF_V8U8
    GL_RGBA8_SNORM,            // VF_Q8W8V8U8
    GL_RG16_SNORM,             // VF_V16U16
    GL_RGBA16_SNORM,           // VF_Q16W16V16U16
    GL_R16F,                   // VF_R16F
    GL_RG16F,                  // VF_G16FR16F
    GL_RGBA16F,                // VF_A16FB16FG16FR16F
    GL_R32F,                   // VF_R32F
    GL_RG32F,                  // VF_G32FR32F
    GL_RGBA32F,                // VF_A32FB32FG32FR32F
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,   // VF_DXT1
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,  // VF_DXT2
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,  // VF_DXT3
    GL_COMPRESSED_RED_RGTC1,           // VF_DXT4
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,  // VF_DXT5
    
    0, // VF_ATI2
    0, // VF_Count
    0, // VF_Invalid
};

RenderDevice::RenderDevice()
    : frameIndex( 0 )
{

}

RenderDevice::~RenderDevice()
{

}

void RenderDevice::initialize()
{
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
        OTDU_LOG_ERROR( "Fatal: Failed to initialize glew '%s'\n", glewGetErrorString(err));
        OTDU_ASSERT_FATAL(false);
    }

    OTDU_LOG_DEBUG("OpenGL Version: '%s'\n", glGetString(GL_VERSION));
    OTDU_LOG_DEBUG("GLSL Version: '%s'\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    OTDU_LOG_DEBUG("GPU Vendor: '%s'\n", glGetString(GL_VENDOR));
    OTDU_LOG_DEBUG("Renderer: '%s'\n", glGetString(GL_RENDERER));
}

GPUTexture* RenderDevice::createTexture( const struct GPUTextureDesc* desc )
{
    const bool bVolumeTex = (desc->Flags >> 0xb & 1) == 1;
    const bool bCubemap = (desc->Flags >> 6 & 1) == 1;

    uint32_t formatGL = GL_TEXTURE_2D;
    if (bVolumeTex)
        formatGL = GL_TEXTURE_3D;
    else if (bCubemap)
        formatGL = GL_TEXTURE_CUBE_MAP;
    
    GPUTexture* renderTarget = new GPUTexture();
    renderTarget->NativeTextureFormat = formatGL;
    
    glGenTextures(1, &renderTarget->NativeTexture);
    OTDU_ASSERT( glGetError() == 0 );

    glBindTexture(renderTarget->NativeTextureFormat, renderTarget->NativeTexture); 
    switch(renderTarget->NativeTextureFormat) {
    case GL_TEXTURE_2D:
        glTexImage2D(GL_TEXTURE_2D, 0, kViewFormatLUT[desc->Format], desc->Width, desc->Height, 0, kViewFormatPixelFormatLUT[desc->Format], kViewFormatTexelFormatLUT[desc->Format], nullptr);
        break;
    default:
        OTDU_UNIMPLEMENTED;
        break;
    }
    glBindTexture(renderTarget->NativeTextureFormat, 0); 

    return renderTarget;
}

GPUBuffer *RenderDevice::createBuffer( const struct GPUBufferDesc* desc )
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

void* RenderDevice::lockBuffer(GPUBuffer *pBuffer, const uint32_t offset, const uint32_t size)
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

void RenderDevice::unlockBuffer(GPUBuffer * pBuffer)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::generateMipchain(GPUTexture *pTexture)
{
    glBindTexture(pTexture->NativeTextureFormat, pTexture->NativeTexture);
    glGenerateMipmap(pTexture->NativeTextureFormat);
    glBindTexture(pTexture->NativeTextureFormat, 0);
}

void RenderDevice::destroyTexture(GPUTexture *pTexture)
{
    glDeleteTextures(1, &pTexture->NativeTexture);
}

void RenderDevice::destroyBuffer(GPUBuffer *pBuffer)
{
    glDeleteBuffers(1, &pBuffer->NativeBuffer);
}
#endif
