#pragma once

#include "d3d9_caps.h"
// #include "d3d9_constant_set.h"
#include "../dxso/dxso_common.h"
#include "../util/util_matrix.h"

//#include "d3d9_surface.h"
//#include "d3d9_shader.h"
//#include "d3d9_vertex_declaration.h"
//#include "d3d9_buffer.h"

#include <array>
#include <bitset>
#include <optional>

namespace dxvk {
  #define DXVK_TSS_COUNT 32
  
typedef enum _D3DSAMPLERSTATETYPE {
    D3DSAMP_ADDRESSU       = 1,
    D3DSAMP_ADDRESSV       = 2,
    D3DSAMP_ADDRESSW       = 3,
    D3DSAMP_BORDERCOLOR    = 4,
    D3DSAMP_MAGFILTER      = 5,
    D3DSAMP_MINFILTER      = 6,
    D3DSAMP_MIPFILTER      = 7,
    D3DSAMP_MIPMAPLODBIAS  = 8,
    D3DSAMP_MAXMIPLEVEL    = 9,
    D3DSAMP_MAXANISOTROPY  = 10,
    D3DSAMP_SRGBTEXTURE    = 11,
    D3DSAMP_ELEMENTINDEX   = 12,
    D3DSAMP_DMAPOFFSET     = 13,
                                
    D3DSAMP_FORCE_DWORD   = 0x7fffffff,
} D3DSAMPLERSTATETYPE;

  static constexpr uint32_t RenderStateCount  = 256;
  static constexpr uint32_t SamplerStateCount = D3DSAMP_DMAPOFFSET + 1;
  static constexpr uint32_t SamplerCount      = caps::MaxTexturesPS + caps::MaxTexturesVS + 1;
  static constexpr uint32_t TextureStageStateCount = DXVK_TSS_COUNT;

  #ifndef _WIN32
  #define UINT uint32_t
  #define DWORD uint32_t
  #endif
  // namespace hacks::PointSize {
  //   static constexpr DWORD AlphaToCoverageDisabled = MAKEFOURCC('A', '2', 'M', '0');
  //   static constexpr DWORD AlphaToCoverageEnabled  = MAKEFOURCC('A', '2', 'M', '1');
  // }
  
  struct D3D9ClipPlane {
    float coeff[4] = {};

    bool operator == (const D3D9ClipPlane& other) {
      return std::memcmp(this, &other, sizeof(D3D9ClipPlane)) == 0;
    }

    bool operator != (const D3D9ClipPlane& other) {
      return !this->operator == (other);
    }
  };

  struct D3D9RenderStateInfo {
    std::array<float, 3> fogColor = { };
    float fogScale   = 0.0f;
    float fogEnd     = 1.0f;
    float fogDensity = 1.0f;

    uint32_t alphaRef = 0u;

    float pointSize    = 1.0f;
    float pointSizeMin = 1.0f;
    float pointSizeMax = 64.0f;
    float pointScaleA  = 1.0f;
    float pointScaleB  = 0.0f;
    float pointScaleC  = 0.0f;
  };

  enum class D3D9RenderStateItem {
    FogColor   = 0,
    FogScale   = 1,
    FogEnd,
    FogDensity,
    AlphaRef,

    PointSize,
    PointSizeMin,
    PointSizeMax,
    PointScaleA,
    PointScaleB,
    PointScaleC,

    Count
  };


  // This is needed in fixed function for POSITION_T support.
  // These are constants we need to * and add to move
  // Window Coords -> Real Coords w/ respect to the viewport.
  struct D3D9ViewportInfo {
    Vector4 inverseOffset;
    Vector4 inverseExtent;
  };

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                 = 0,
    D3DFOG_EXP                  = 1,
    D3DFOG_EXP2                 = 2,
    D3DFOG_LINEAR               = 3,

    D3DFOG_FORCE_DWORD          = 0x7fffffff
} D3DFOGMODE;

typedef enum _D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE            = 0,
    D3DTEXF_POINT           = 1,
    D3DTEXF_LINEAR          = 2,
    D3DTEXF_ANISOTROPIC     = 3,
    D3DTEXF_FLATCUBIC       = 4,
    D3DTEXF_GAUSSIANCUBIC   = 5,
    D3DTEXF_PYRAMIDALQUAD   = 6,
    D3DTEXF_GAUSSIANQUAD    = 7,
#ifndef D3D_DISABLE_9EX
    D3DTEXF_CONVOLUTIONMONO = 8,
#endif
    D3DTEXF_FORCE_DWORD     = 0x7fffffff
} D3DTEXTUREFILTERTYPE;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP            = 1,
    D3DTADDRESS_MIRROR          = 2,
    D3DTADDRESS_CLAMP           = 3,
    D3DTADDRESS_BORDER          = 4,
    D3DTADDRESS_MIRRORONCE      = 5,

    D3DTADDRESS_FORCE_DWORD     = 0x7fffffff
} D3DTEXTUREADDRESS;

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif

  typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,

    D3DLIGHT_FORCE_DWORD    = 0x7fffffff
  } D3DLIGHTTYPE;

  typedef struct _D3DLIGHT9 {
      D3DLIGHTTYPE    Type;
      D3DCOLORVALUE   Diffuse;
      D3DCOLORVALUE   Specular;
      D3DCOLORVALUE   Ambient;
      D3DVECTOR       Position;
      D3DVECTOR       Direction;
      float           Range;
      float           Falloff;
      float           Attenuation0;
      float           Attenuation1;
      float           Attenuation2;
      float           Theta;
      float           Phi;
  } D3DLIGHT9;

  struct D3D9Light {
    D3D9Light(const D3DLIGHT9& light, Matrix4 viewMtx)
      : Diffuse      ( Vector4(light.Diffuse.r,  light.Diffuse.g,  light.Diffuse.b,  light.Diffuse.a) )
      , Specular     ( Vector4(light.Specular.r, light.Specular.g, light.Specular.b, light.Specular.a) )
      , Ambient      ( Vector4(light.Ambient.r,  light.Ambient.g,  light.Ambient.b,  light.Ambient.a) )
      , Position     ( viewMtx * Vector4(light.Position.x,  light.Position.y,  light.Position.z,  1.0f) )
      , Direction    ( normalize(viewMtx * Vector4(light.Direction.x, light.Direction.y, light.Direction.z, 0.0f)) )
      , Type         ( light.Type )
      , Range        ( light.Range )
      , Falloff      ( light.Falloff )
      , Attenuation0 ( light.Attenuation0 )
      , Attenuation1 ( light.Attenuation1 )
      , Attenuation2 ( light.Attenuation2 )
      , Theta        ( cosf(light.Theta / 2.0f) )
      , Phi          ( cosf(light.Phi / 2.0f) ) { }

    Vector4 Diffuse;
    Vector4 Specular;
    Vector4 Ambient;

    Vector4 Position;
    Vector4 Direction;

    D3DLIGHTTYPE Type;
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
  };

typedef struct _D3DMATERIAL9 {
    D3DCOLORVALUE   Diffuse;
    D3DCOLORVALUE   Ambient;
    D3DCOLORVALUE   Specular;
    D3DCOLORVALUE   Emissive;
    float           Power;
} D3DMATERIAL9;

  struct D3D9FixedFunctionVS {
    Matrix4 WorldView;
    Matrix4 NormalMatrix;
    Matrix4 InverseView;
    Matrix4 Projection;

    std::array<Matrix4, 8> TexcoordMatrices;

    D3D9ViewportInfo ViewportInfo;

    Vector4 GlobalAmbient;
    std::array<D3D9Light, caps::MaxEnabledLights> Lights;
    D3DMATERIAL9 Material;
    float TweenFactor;
  };


  struct D3D9FixedFunctionVertexBlendDataHW {
    Matrix4 WorldView[8];
  };


  struct D3D9FixedFunctionVertexBlendDataSW {
    Matrix4 WorldView[256];
  };


  struct D3D9FixedFunctionPS {
    Vector4 textureFactor;
  };

  enum D3D9SharedPSStages {
    D3D9SharedPSStages_Constant,
    D3D9SharedPSStages_BumpEnvMat0,
    D3D9SharedPSStages_BumpEnvMat1,
    D3D9SharedPSStages_BumpEnvLScale,
    D3D9SharedPSStages_BumpEnvLOffset,
    D3D9SharedPSStages_Count,
  };

  struct D3D9SharedPS {
    struct Stage {
      float Constant[4];
      float BumpEnvMat[2][2];
      float BumpEnvLScale;
      float BumpEnvLOffset;
      float Padding[2];
    } Stages[8];
  };
  
  // struct D3D9VBO {
  //   Com<D3D9VertexBuffer, false> vertexBuffer;

  //   UINT              offset = 0;
  //   UINT              stride = 0;
  // };

  constexpr D3DLIGHT9 DefaultLight = {
    D3DLIGHT_DIRECTIONAL,     // Type
    {1.0f, 1.0f, 1.0f, 0.0f}, // Diffuse
    {0.0f, 0.0f, 0.0f, 0.0f}, // Specular
    {0.0f, 0.0f, 0.0f, 0.0f}, // Ambient
    {0.0f, 0.0f, 0.0f},       // Position
    {0.0f, 0.0f, 1.0f},       // Direction
    0.0f,                     // Range
    0.0f,                     // Falloff
    0.0f, 0.0f, 0.0f,         // Attenuations [constant, linear, quadratic]
    0.0f,                     // Theta
    0.0f                      // Phi
  };

  template <typename T>
  class dynamic_item {
  public:
          auto& operator [] (size_t idx)       { ensure(); return (*m_data)[idx]; }
    const auto& operator [] (size_t idx) const { ensure(); return (*m_data)[idx]; }

    T& operator=(const T& x) { ensure(); *m_data = x; return *m_data; }

    const T* operator -> () const { ensure(); return m_data.get(); }
          T* operator -> ()       { ensure(); return m_data.get(); }

    const T* operator & () const { ensure(); return m_data.get(); }
          T* operator & ()       { ensure(); return m_data.get(); }

    explicit operator bool() const { return m_data != nullptr; }
    operator T() { ensure(); return *m_data; }

    void ensure() const { if (!m_data) m_data = std::make_unique<T>(); }

    T& get() { ensure(); return *m_data; }
  private:
    mutable std::unique_ptr<T> m_data;
  };

  template <typename T>
  class static_item {
  public:
          auto& operator [] (size_t idx)       { return m_data[idx]; }
    const auto& operator [] (size_t idx) const { return m_data[idx]; }

    T& operator=(const T& x) { m_data = x; return m_data; }

    explicit operator bool() const { return true; }
    operator T() { return m_data; }

    const T* operator -> () const { return &m_data; }
          T* operator -> ()       { return &m_data; }

    const T* operator & () const { return &m_data; }
          T* operator & ()       { return &m_data; }

    T& get() { return m_data; }
  private:
    T m_data;
  };

  struct D3D9SamplerInfo {
    D3D9SamplerInfo(const std::array<DWORD, SamplerStateCount>& state)
    : addressU(D3DTEXTUREADDRESS(state[D3DSAMP_ADDRESSU]))
    , addressV(D3DTEXTUREADDRESS(state[D3DSAMP_ADDRESSV]))
    , addressW(D3DTEXTUREADDRESS(state[D3DSAMP_ADDRESSW]))
    , borderColor(D3DCOLOR(state[D3DSAMP_BORDERCOLOR]))
    , magFilter(D3DTEXTUREFILTERTYPE(state[D3DSAMP_MAGFILTER]))
    , minFilter(D3DTEXTUREFILTERTYPE(state[D3DSAMP_MINFILTER]))
    , mipFilter(D3DTEXTUREFILTERTYPE(state[D3DSAMP_MIPFILTER]))
    , mipLodBias(bit::cast<float>(state[D3DSAMP_MIPMAPLODBIAS]))
    , maxMipLevel(state[D3DSAMP_MAXMIPLEVEL])
    , maxAnisotropy(state[D3DSAMP_MAXANISOTROPY]) { }

    D3DTEXTUREADDRESS addressU;
    D3DTEXTUREADDRESS addressV;
    D3DTEXTUREADDRESS addressW;
    D3DCOLOR borderColor;
    D3DTEXTUREFILTERTYPE magFilter;
    D3DTEXTUREFILTERTYPE minFilter;
    D3DTEXTUREFILTERTYPE mipFilter;
    float mipLodBias;
    DWORD maxMipLevel;
    DWORD maxAnisotropy;
  };

  // template <template <typename T> typename ItemType>
  // struct D3D9State {
  //   D3D9State();
  //   ~D3D9State();

  //   Com<D3D9VertexDecl,  false>                         vertexDecl;
  //   Com<D3D9IndexBuffer, false>                         indices;

  //   ItemType<std::array<DWORD, RenderStateCount>>       renderStates = {};

  //   ItemType<std::array<
  //     std::array<DWORD, SamplerStateCount>,
  //     SamplerCount>>                                    samplerStates = {};

  //   ItemType<std::array<D3D9VBO, caps::MaxStreams>>     vertexBuffers = {};

  //   ItemType<std::array<
  //     IDirect3DBaseTexture9*,
  //     SamplerCount>>                                    textures = {};

  //   Com<D3D9VertexShader, false>                        vertexShader;
  //   Com<D3D9PixelShader,  false>                        pixelShader;

  //   D3DVIEWPORT9                                        viewport = {};
  //   RECT                                                scissorRect = {};

  //   D3DCLIPSTATUS9                                      clipStatus = {0, 0xffffffff};

  //   ItemType<std::array<
  //     D3D9ClipPlane,
  //     caps::MaxClipPlanes>>                             clipPlanes = {};

  //   ItemType<std::array<
  //     std::array<DWORD, TextureStageStateCount>,
  //     caps::TextureStageCount>>                         textureStages = {};

  //   ItemType<D3D9ShaderConstantsVSSoftware>             vsConsts;
  //   ItemType<D3D9ShaderConstantsPS>                     psConsts;

  //   std::array<UINT, caps::MaxStreams>                  streamFreq = {};

  //   ItemType<std::array<Matrix4, caps::MaxTransforms>>  transforms = {};

  //   ItemType<D3DMATERIAL9>                              material = {};

  //   std::vector<std::optional<D3DLIGHT9>>               lights;
  //   std::array<DWORD, caps::MaxEnabledLights>           enabledLightIndices;

  //   bool IsLightEnabled(DWORD Index) const {
  //     const auto& enabledIndices = enabledLightIndices;
  //     return std::find(enabledIndices.begin(), enabledIndices.end(), Index) != enabledIndices.end();
  //   }
  // };

  // using D3D9CapturableState = D3D9State<dynamic_item>;
  // using D3D9DeviceState = D3D9State<static_item>;

  // template <
  //   DxsoProgramType  ProgramType,
  //   D3D9ConstantType ConstantType,
  //   typename         T,
  //   typename         StateType>
  // HRESULT UpdateStateConstants(
  //         StateType*           pState,
  //         UINT                 StartRegister,
  //   const T*                   pConstantData,
  //         UINT                 Count,
  //         bool                 FloatEmu) {
  //   auto UpdateHelper = [&] (auto& set) {
  //     if constexpr (ConstantType == D3D9ConstantType::Float) {

  //       if (!FloatEmu) {
  //         size_t size = Count * sizeof(Vector4);

  //         std::memcpy(set->fConsts[StartRegister].data, pConstantData, size);
  //       }
  //       else {
  //         for (UINT i = 0; i < Count; i++)
  //           set->fConsts[StartRegister + i] = replaceNaN(pConstantData + (i * 4));
  //       }
  //     }
  //     else if constexpr (ConstantType == D3D9ConstantType::Int) {
  //       size_t size = Count * sizeof(Vector4i);

  //       std::memcpy(set->iConsts[StartRegister].data, pConstantData, size);
  //     }
  //     else {
  //       for (uint32_t i = 0; i < Count; i++) {
  //         const uint32_t constantIdx = StartRegister + i;
  //         const uint32_t arrayIdx    = constantIdx / 32;
  //         const uint32_t bitIdx      = constantIdx % 32;

  //         const uint32_t bit = 1u << bitIdx;

  //         set->bConsts[arrayIdx] &= ~bit;
  //         if (pConstantData[i])
  //           set->bConsts[arrayIdx] |= bit;
  //       }
  //     }

  //     return D3D_OK;
  //   };

  //   return ProgramType == DxsoProgramTypes::VertexShader
  //     ? UpdateHelper(pState->vsConsts)
  //     : UpdateHelper(pState->psConsts);
  // }

  // struct Direct3DState9 : public D3D9DeviceState {

  //   std::array<Com<D3D9Surface, false>, caps::MaxSimultaneousRenderTargets> renderTargets;
  //   Com<D3D9Surface, false> depthStencil;

  // };


  // struct D3D9InputAssemblyState {
  //   D3DPRIMITIVETYPE primitiveType = D3DPRIMITIVETYPE(0);
  //   uint32_t streamsInstanced = 0;
  //   uint32_t streamsUsed      = 0;
  // };

}