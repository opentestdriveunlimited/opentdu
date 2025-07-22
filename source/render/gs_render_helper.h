#pragma once

enum class eAntiAliasingMethod : uint32_t {
    AAM_Disabled = 0,
    AAM_MSAA_X2,
    AAM_MSAA_X4
};

enum class eVSyncMode {
    VSM_Immediate = 0,
    VSM_OneVBlank = 1,
    VSM_TwoVBlank = 2,
    VSM_ThreeVBank = 3,
    VSM_FourVBlank = 4
};

enum class eShaderType {
    ST_Pixel = 0,
    ST_Vertex,

    ST_Count,

    ST_Invalid = -1
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
    VF_A16B16G16R16F = 19,
    VF_R32F = 20,
    VF_G32R32F = 21,
    VF_A32B32G32R32F = 22,
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
    VF_SRGB_G32R32F = 49,
    VF_SRGB_A32B32G32R32F = 50,
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
    VF_D16 = 24,
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

struct GPUAdapterDesc {
    std::string AdapterName = "";
    std::string DeviceDesc = "";
    uint32_t DeviceID = 0xffffffff;
    uint32_t SubSysId = 0xffffffff;
    uint32_t Revision = 0xffffffff;
    uint32_t VendorId = 0xffffffff;
};

static constexpr uint32_t kMaxSimultaneousRT = 8;

static bool IsBlockCompressedFormat( eViewFormat format )
{
    return format == eViewFormat::VF_DXT1
        || format == eViewFormat::VF_DXT2
        || format == eViewFormat::VF_DXT3
        || format == eViewFormat::VF_DXT4
        || format == eViewFormat::VF_DXT5
        || format == eViewFormat::VF_SRGB_DXT1
        || format == eViewFormat::VF_SRGB_DXT2
        || format == eViewFormat::VF_SRGB_DXT3
        || format == eViewFormat::VF_SRGB_DXT4
        || format == eViewFormat::VF_SRGB_DXT5
        || format == eViewFormat::VF_ATI2;
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

#ifdef OTDU_D3D9
#include "d3d9/render_device.h"
#elif defined(OTDU_OPENGL)
#include "opengl/render_device.h"
#else
static_assert(false, "Unimplemented API/missing preprocessor!");
#endif
