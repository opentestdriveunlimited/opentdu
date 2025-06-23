#pragma once

// TODO: This header is a mess; do a cleanup
enum class eAntiAliasingMethod {
    AAM_Disabled = 0,
    AAM_MSAA_X2,
    AAM_MSAA_X4
};

enum class eShaderStage : uint32_t {
    SS_Vertex = 0,
    SS_Pixel = 1,

    SS_Count,

    SS_Invalid = 0xffffffff
};

enum eViewFormat : uint32_t {
    VF_R8G8B8 = 0,
    VF_R8G8B8A8 = 1,
    VF_X8R8G8B8 = 2,
    VF_A8R8G8B8 = 3,
    VF_R5G6B5 = 4,
    VF_A1R5G5B5 = 5,
    VF_A8 = 6,
    VF_A8R3G3B2 = 7,
    VF_A2R10G10B10 = 8,
    VF_G16R16 = 9,
    VF_A16B16G16R16 = 10,
    VF_L8 = 11,
    VF_L16 = 12,
    VF_V8U8 = 13,
    VF_Q8W8V8U8 = 14,
    VF_V16U16 = 15,
    VF_Q16W16V16U16 = 16,
    VF_R16F = 17,
    VF_G16FR16F = 18,
    VF_A16FB16FG16FR16F = 19,
    VF_R32F = 20,
    VF_G32FR32F = 21,
    VF_A32FB32FG32FR32F = 22,
    VF_DXT1 = 23,
    VF_DXT2 = 24,
    VF_DXT3 = 25,
    VF_DXT4 = 26,
    VF_DXT5 = 27,
    VF_SRGB_R8G8B8 = 28,
    VF_SRGB_R8G8B8A8 = 29,
    VF_SRGB_X8R8G8B8 = 30,
    VF_SRGB_A8R8G8B8 = 31,
    VF_SRGB_R5G6B5 = 32,
    VF_SRGB_A1R5G5B5 = 33,
    VF_SRGB_A8 = 34,
    VF_SRGB_A8R3G3B2 = 35,
    VF_SRGB_A2R10G10B10 = 36,
    VF_SRGB_G16R16 = 37,
    VF_SRGB_A16B16G16R16 = 38,
    VF_SRGB_L8 = 39,
    VF_SRGB_L16 = 40,
    VF_SRGB_V8U8 = 41,
    VF_SRGB_Q8W8V8U8 = 42,
    VF_SRGB_V16U16 = 43,
    VF_SRGB_Q16W16V16U16 = 44,
    VF_SRGB_R16F = 45,
    VF_SRGB_G16FR16F = 46,
    VF_SRGB_A16FB16FG16FR16F = 47,
    VF_SRGB_R32F = 48,
    VF_SRGB_G32FR32F = 49,
    VF_SRGB_A32FB32FG32FR32F = 50,
    VF_SRGB_DXT1 = 51,
    VF_SRGB_DXT2 = 52,
    VF_SRGB_DXT3 = 53,
    VF_SRGB_DXT4 = 54,
    VF_SRGB_DXT5 = 55,
    VF_ATI2 = 61,
    VF_Count = 62,
    VF_Invalid = 0xffffffff,

    // Depth/Stencil formats
    VF_D24S8 = 22,
    VF_D24S8F = 23,
    VF_D16 = 24
};

enum eRenderTargetFlags : uint32_t {
    // TODO: Incomplete
    RTF_None = 0,
    RTF_Cubemap = 1 << 6,
    RTF_ReadOnly = 1 << 9,
    RTF_Volume = 1 << 11,
    RTF_GenerateMips = 1 << 0xe,
};

enum class ePrimitiveType {
    PT_Point = 0,
    PT_Line = 1,
    PT_LineStrip = 2,
    PT_Triangle = 3,
    PT_TriangleStrip = 4,
    PT_TriangleFan = 5,
    PT_Quad = 6,
    PT_HMap = 7,
    PT_Count
};

// From the decompiled source; probably need some trim/upgrading for modern APIs
struct GPUTextureDesc 
{
    uint16_t Width = 0u;
    uint16_t Height = 0u;
    uint16_t Depth = 1u;
    uint16_t ArrayLength = 1u;
    uint16_t MipCount = 1u;
    eViewFormat Format = eViewFormat::VF_Invalid;
    uint32_t SampleCount = 1u; // <= 1 if not multisampled/no MSAA
    uint32_t Flags = eRenderTargetFlags::RTF_None; // Combination of eRenderTargetFlags
    uint64_t Hashcode = 0ull; // Reversed from the original binary; not really used by OTDU atm
    uint32_t Size = 0u; // Same as Hashcode

    GPUTextureDesc( uint32_t width,
                      uint32_t height,
                      uint32_t depth,
                      uint32_t numMips,
                      eViewFormat format,
                      uint32_t flags,
                      const char* pResourceName = nullptr )
    {
        Fill( width, height, depth, numMips, format, flags, pResourceName );
    }

    // Helper to fill a GPUTextureDesc.
    void Fill( uint32_t width, uint32_t height, uint32_t depth, uint32_t numMips,
               eViewFormat format, uint32_t flags, const char* pResourceName = nullptr  );
};

static bool IsBlockCompressFormat( eViewFormat format )
{
    return format == eViewFormat::VF_DXT1
        || format == eViewFormat::VF_DXT2
        || format == eViewFormat::VF_DXT3
        || format == eViewFormat::VF_DXT4
        || format == eViewFormat::VF_DXT5
        || format == eViewFormat::VF_ATI2;
}

static uint32_t CalcPitch( uint32_t width, eViewFormat format )
{
    uint32_t uVar1 = width * 4;

    OTDU_UNIMPLEMENTED;

    //if ( IsBlockCompressFormat( format ) ) {
    //    if ( format == eViewFormat::VF_DXT1 ) {
    //        return uVar1 >> 3;
    //    }

    //    if ( ( ( format != eViewFormat::VF_ATI2 )
    //           && ( format != eViewFormat::VF_DXT3 ) )
    //         && ( format != eViewFormat::VF_DXT5 ) ) {
    //        return uVar1;
    //    }

    //    return width & 0x3fffffff;
    //}

    //if ( ( format != eViewFormat::VF_A16B16G16R16F ) && ( format != eViewFormat::VF_G32R32F ) ) {
    //    if ( format != eViewFormat::VF_A32B32G32R32F ) {
    //        return uVar1;
    //    }

    //    uVar1 = width * 8;
    //    return uVar1 * 2;
    //}

    //if ( format != eViewFormat::VF_R16F ) {
    //    switch ( format ) {
    //    case eViewFormat::VF_R5G6B5:
    //    case eViewFormat::VF_A1R5G5B5:
    //    case eViewFormat::VF_D15S1:
    //    case eViewFormat::VF_D16:
    //    case eViewFormat::VF_L16:
    //        break;
    //    default:
    //        return uVar1;
    //    case eViewFormat::VF_A16B16G16R16:
    //    case eViewFormat::VF_Q16W16V16U16:
    //        return uVar1 * 2;
    //    case eViewFormat::VF_L8:
    //        return width & 0x3fffffff;
    //    }
    //}

    return uVar1 >> 1;
}

static int32_t CalcMipSize( int32_t width, int32_t height, eViewFormat format )
{
    return CalcPitch( width, format ) * height;
}

static uint32_t CalcRenderTargetSize( const GPUTextureDesc& desc )
{
    uint32_t texSize = 0;
    if ( desc.Width != 0 && desc.Height != 0 && desc.MipCount != 0 ) {
        uint32_t mipWidth = desc.Width;
        uint32_t mipHeight = desc.Height;
        for ( int32_t mipIdx = desc.MipCount; mipIdx != 0; mipIdx-- ) {
            uint32_t mipSize = CalcMipSize( mipWidth, mipHeight, desc.Format );
            texSize += mipSize;

            mipWidth = Max( mipWidth >> 1, 1 );
            mipHeight = Max( mipHeight >> 1, 1 );
        }
    }

    uint32_t arrayLength = 1u;
    if ( ( desc.Flags & RTF_Volume ) != 0 ) {
        OTDU_ASSERT( desc.ArrayLength >= 1 );
        arrayLength = desc.ArrayLength;
    }
    if ( ( desc.Flags & RTF_Cubemap ) != 0 ) {
        arrayLength = 6u;
    }

    texSize *= arrayLength;

    return texSize;
}

static uint32_t CalcResourceSize(
    uint32_t width, uint32_t height, uint32_t depth,
    uint32_t numMips,
    eViewFormat format,
    uint32_t flags )
{
    if ( ( flags & RTF_ReadOnly ) != 0 ) {
        return 0;
    }

    GPUTextureDesc desc( width, height, depth, numMips, format, flags );
    return CalcRenderTargetSize( desc ) + 15U & 0xfffffff0;
}

static int32_t CalculateResourceSize(
    uint32_t width, uint32_t height, uint32_t depth,
    uint32_t numMips,
    eViewFormat format,
    uint32_t flags )
{
    if ( ( flags & RTF_ReadOnly ) != 0 ) {
        return 80;
    }

    return CalcResourceSize( width, height, depth, numMips, format, flags ) + 95 & 0xfffffff0;
}

static constexpr const uint32_t kVertexComponentSizeLUT[0x10] = {
    4,
    2,
    4,
    4,
    4,
    2,
    4,
    0,
    0,
    4,
    2,
    2,
    4,
    2,
    2,
    4
};
  
static constexpr const uint32_t kVertexComponentStrideLUT[0x10] = {
   3,
   4,
   1,
   1,
   4,
   4,
   3,
   0,
   0,
   1,
   4,
   4,
   2,
   2,
   2,
   1
};

static constexpr uint32_t GetVertexAttributeSize(const uint32_t attribute)
{
    return kVertexComponentStrideLUT[attribute] * kVertexComponentSizeLUT[attribute];
}

struct GPUBufferDesc {
    enum class Type {
        BT_Unknown,
        BT_VertexBuffer,
        BT_IndexBuffer
    };

    Type        BufferType;
    uint32_t    Size;

    union {
        eViewFormat Format;
        uint32_t    Stride;
    };

    uint8_t bDynamic : 1;

    GPUBufferDesc()
        : BufferType( GPUBufferDesc::Type::BT_Unknown )
        , Size( 0u )
        , Stride( 0u )
        , bDynamic( false )
    {
        
    }
};

#ifdef OTDU_VULKAN
#include "vulkan/render_device.h"
#elif defined(OTDU_OPENGL)
#include "opengl/render_device.h"
#else
static_assert(false, "Unimplemented API/missing preprocessor!");
#endif
