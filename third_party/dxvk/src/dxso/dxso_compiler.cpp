#include "dxso_compiler.h"

#include "dxso_analysis.h"

#include "../d3d9/d3d9_caps.h"
//#include "../d3d9/d3d9_constant_set.h"
#include "../d3d9/d3d9_state.h"
#include "../d3d9/d3d9_spec_constants.h"
#include "../d3d9/d3d9_fixed_function.h"
#include "dxso_util.h"

#include "core/logger.h"

#include <cfloat>

namespace dxvk
{
  /**
   * @brief The first sampler that belongs to the vertex shader according to our internal way of storing samplers
   */
  constexpr uint32_t FirstVSSamplerSlot = caps::MaxTexturesPS + 1;

    uint32_t _SetupSamplerArray( SpirvModule& spvModule )
    {
        // Old spir-v, need to enable extension
        spvModule.enableExtension( "SPV_EXT_descriptor_indexing" );
        spvModule.enableCapability( spv::CapabilityRuntimeDescriptorArray );

        uint32_t samplerArray = spvModule.defRuntimeArrayTypeUnique( spvModule.defSamplerType() );
        uint32_t samplerPtr = spvModule.defPointerType( samplerArray, spv::StorageClassUniformConstant );

        uint32_t samplerHeap = spvModule.newVar( samplerPtr, spv::StorageClassUniformConstant );
        spvModule.setDebugName( samplerHeap, "sampler_heap" );

        spvModule.decorateBinding( samplerHeap, 0u );
        spvModule.decorateDescriptorSet( samplerHeap, GetGlobalSamplerSetIndex() );
        return samplerHeap;
    }

    uint32_t _LoadSampler( SpirvModule& spvModule, uint32_t descriptorId,
                          uint32_t pushBlockId, uint32_t pushMember, uint32_t samplerIndex )
    {
        uint32_t uintType = spvModule.defIntType( 32u, 0 );
        uint32_t uintPtr = spvModule.defPointerType( uintType, spv::StorageClassPushConstant );

        uint32_t pushIndexId = spvModule.constu32( pushMember + samplerIndex / 2u );

        uint32_t descriptorIndex = spvModule.opLoad( uintType,
                                                     spvModule.opAccessChain( uintPtr, pushBlockId, 1u, &pushIndexId ) );

        descriptorIndex = spvModule.opBitFieldUExtract( uintType, descriptorIndex,
                                                        spvModule.constu32( 16u * ( samplerIndex & 1u ) ), spvModule.constu32( 16u ) );

        uint32_t samplerType = spvModule.defSamplerType();
        uint32_t samplerPtr = spvModule.defPointerType( samplerType, spv::StorageClassUniformConstant );

        return spvModule.opLoad( samplerType,
                                 spvModule.opAccessChain( samplerPtr, descriptorId, 1u, &descriptorIndex ) );
    }

    uint32_t _GetSharedConstants( SpirvModule& spvModule )
    {
        uint32_t float_t = spvModule.defFloatType( 32 );
        uint32_t vec2_t = spvModule.defVectorType( float_t, 2 );
        uint32_t vec4_t = spvModule.defVectorType( float_t, 4 );

        std::array<uint32_t, D3D9SharedPSStages_Count> stageMembers = {
          vec4_t,

          vec2_t,
          vec2_t,

          float_t,
          float_t,
        };

        std::array<decltype( stageMembers ), caps::TextureStageCount> members;

        for ( auto& member : members )
            member = stageMembers;

        const uint32_t structType =
            spvModule.defStructType( members.size() * stageMembers.size(), members[0].data() );

        spvModule.decorateBlock( structType );

        uint32_t offset = 0;
        for ( uint32_t stage = 0; stage < caps::TextureStageCount; stage++ ) {
            spvModule.memberDecorateOffset( structType, stage * D3D9SharedPSStages_Count + D3D9SharedPSStages_Constant, offset );
            offset += sizeof( float ) * 4;

            spvModule.memberDecorateOffset( structType, stage * D3D9SharedPSStages_Count + D3D9SharedPSStages_BumpEnvMat0, offset );
            offset += sizeof( float ) * 2;

            spvModule.memberDecorateOffset( structType, stage * D3D9SharedPSStages_Count + D3D9SharedPSStages_BumpEnvMat1, offset );
            offset += sizeof( float ) * 2;

            spvModule.memberDecorateOffset( structType, stage * D3D9SharedPSStages_Count + D3D9SharedPSStages_BumpEnvLScale, offset );
            offset += sizeof( float );

            spvModule.memberDecorateOffset( structType, stage * D3D9SharedPSStages_Count + D3D9SharedPSStages_BumpEnvLOffset, offset );
            offset += sizeof( float );

            // Padding...
            offset += sizeof( float ) * 2;
        }

        uint32_t sharedState = spvModule.newVar(
            spvModule.defPointerType( structType, spv::StorageClassUniform ),
            spv::StorageClassUniform );

        spvModule.setDebugName( sharedState, "D3D9SharedPS" );

        return sharedState;
    }

    uint32_t _GetPointCoord( SpirvModule& spvModule )
    {
        uint32_t floatType = spvModule.defFloatType( 32 );
        uint32_t vec2Type = spvModule.defVectorType( floatType, 2 );
        uint32_t vec4Type = spvModule.defVectorType( floatType, 4 );
        uint32_t vec2Ptr = spvModule.defPointerType( vec2Type, spv::StorageClassInput );

        uint32_t pointCoordPtr = spvModule.newVar( vec2Ptr, spv::StorageClassInput );

        spvModule.decorateBuiltIn( pointCoordPtr, spv::BuiltInPointCoord );

        uint32_t pointCoord = spvModule.opLoad( vec2Type, pointCoordPtr );

        std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };

        std::array<uint32_t, 4> pointCoordIndices = {
          spvModule.opCompositeExtract( floatType, pointCoord, 1, &indices[0] ),
          spvModule.opCompositeExtract( floatType, pointCoord, 1, &indices[1] ),
          spvModule.constf32( 0.0f ),
          spvModule.constf32( 0.0f )
        };

        return spvModule.opCompositeConstruct( vec4Type, pointCoordIndices.size(), pointCoordIndices.data() );
    }

    D3D9PointSizeInfoVS _GetPointSizeInfoVS( D3D9ShaderSpecConstantManager& spec, SpirvModule& spvModule, uint32_t vPos, uint32_t vtx, uint32_t perVertPointSize, uint32_t rsBlock, uint32_t specUbo, bool isFixedFunction )
    {
        uint32_t floatType = spvModule.defFloatType( 32 );
        uint32_t floatPtr = spvModule.defPointerType( floatType, spv::StorageClassPushConstant );
        uint32_t vec3Type = spvModule.defVectorType( floatType, 3 );
        uint32_t vec4Type = spvModule.defVectorType( floatType, 4 );
        uint32_t uint32Type = spvModule.defIntType( 32, 0 );
        uint32_t boolType = spvModule.defBoolType();

        auto LoadFloat = [&]( D3D9RenderStateItem item ) {
            uint32_t index = spvModule.constu32( uint32_t( item ) );
            return spvModule.opLoad( floatType, spvModule.opAccessChain( floatPtr, rsBlock, 1, &index ) );
        };

        uint32_t value = perVertPointSize != 0 ? perVertPointSize : LoadFloat( D3D9RenderStateItem::PointSize );

        if ( isFixedFunction ) {
            uint32_t pointMode = spec.get( spvModule, specUbo, SpecPointMode );

            uint32_t scaleBit = spvModule.opBitFieldUExtract( uint32Type, pointMode, spvModule.consti32( 0 ), spvModule.consti32( 1 ) );
            uint32_t isScale = spvModule.opIEqual( boolType, scaleBit, spvModule.constu32( 1 ) );

            uint32_t scaleC = LoadFloat( D3D9RenderStateItem::PointScaleC );
            uint32_t scaleB = LoadFloat( D3D9RenderStateItem::PointScaleB );
            uint32_t scaleA = LoadFloat( D3D9RenderStateItem::PointScaleA );

            std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };

            uint32_t vtx3;
            if ( vPos != 0 ) {
                vPos = spvModule.opLoad( vec4Type, vPos );

                uint32_t rhw = spvModule.opCompositeExtract( floatType, vPos, 1, &indices[3] );
                rhw = spvModule.opFDiv( floatType, spvModule.constf32( 1.0f ), rhw );
                uint32_t pos3 = spvModule.opVectorShuffle( vec3Type, vPos, vPos, 3, indices.data() );
                vtx3 = spvModule.opVectorTimesScalar( vec3Type, pos3, rhw );
            } else {
                vtx3 = spvModule.opVectorShuffle( vec3Type, vtx, vtx, 3, indices.data() );
            }

            uint32_t DeSqr = spvModule.opDot( floatType, vtx3, vtx3 );
            uint32_t De = spvModule.opSqrt( floatType, DeSqr );
            uint32_t scaleValue = spvModule.opFMul( floatType, scaleC, DeSqr );
            scaleValue = spvModule.opFFma( floatType, scaleB, De, scaleValue );
            scaleValue = spvModule.opFAdd( floatType, scaleA, scaleValue );
            scaleValue = spvModule.opSqrt( floatType, scaleValue );
            scaleValue = spvModule.opFDiv( floatType, value, scaleValue );

            value = spvModule.opSelect( floatType, isScale, scaleValue, value );
        }

        uint32_t min = LoadFloat( D3D9RenderStateItem::PointSizeMin );
        uint32_t max = LoadFloat( D3D9RenderStateItem::PointSizeMax );

        D3D9PointSizeInfoVS info;
        info.defaultValue = value;
        info.min = min;
        info.max = max;

        return info;
    }

    D3D9PointSizeInfoPS _GetPointSizeInfoPS( D3D9ShaderSpecConstantManager& spec, SpirvModule& spvModule, uint32_t rsBlock, uint32_t specUbo )
    {
        uint32_t uint32Type = spvModule.defIntType( 32, 0 );
        uint32_t boolType = spvModule.defBoolType();
        uint32_t boolVec4 = spvModule.defVectorType( boolType, 4 );

        uint32_t pointMode = spec.get( spvModule, specUbo, SpecPointMode );

        uint32_t spriteBit = spvModule.opBitFieldUExtract( uint32Type, pointMode, spvModule.consti32( 1 ), spvModule.consti32( 1 ) );
        uint32_t isSprite = spvModule.opIEqual( boolType, spriteBit, spvModule.constu32( 1 ) );

        std::array<uint32_t, 4> isSpriteIndices;
        for ( uint32_t i = 0; i < isSpriteIndices.size(); i++ )
            isSpriteIndices[i] = isSprite;

        isSprite = spvModule.opCompositeConstruct( boolVec4, isSpriteIndices.size(), isSpriteIndices.data() );

        D3D9PointSizeInfoPS info;
        info.isSprite = isSprite;

        return info;
    }


    std::pair<uint32_t, uint32_t> _SetupRenderStateBlock( SpirvModule& spvModule, uint32_t samplerMask )
    {
        uint32_t floatType = spvModule.defFloatType( 32 );
        uint32_t uintType = spvModule.defIntType( 32, 0 );
        uint32_t vec3Type = spvModule.defVectorType( floatType, 3 );

        small_vector<uint32_t, 32u> rsMembers;
        rsMembers.push_back( vec3Type );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );

        rsMembers.push_back( uintType );

        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );
        rsMembers.push_back( floatType );

        // Number of static data members
        uint32_t rsMemberCount = rsMembers.size();

        // Add one dword for each sampler pair
        uint32_t samplerCount = bit::popcnt( samplerMask );

        for ( uint32_t i = 0u; i < samplerCount; i += 2u )
            rsMembers.push_back( uintType );

        uint32_t rsStruct = spvModule.defStructTypeUnique( rsMembers.size(), rsMembers.data() );
        uint32_t rsBlock = spvModule.newVar(
            spvModule.defPointerType( rsStruct, spv::StorageClassPushConstant ),
            spv::StorageClassPushConstant );

        spvModule.setDebugName( rsBlock, "render_state" );

        spvModule.setDebugName( rsStruct, "render_state_t" );
        spvModule.decorate( rsStruct, spv::DecorationBlock );

        uint32_t memberIdx = 0;
        auto SetMemberName = [&]( const char* name, uint32_t offset ) {
            spvModule.setDebugMemberName( rsStruct, memberIdx, name );
            spvModule.memberDecorateOffset( rsStruct, memberIdx, offset );
            memberIdx++;
        };

        SetMemberName( "fog_color", offsetof( D3D9RenderStateInfo, fogColor ) );
        SetMemberName( "fog_scale", offsetof( D3D9RenderStateInfo, fogScale ) );
        SetMemberName( "fog_end", offsetof( D3D9RenderStateInfo, fogEnd ) );
        SetMemberName( "fog_density", offsetof( D3D9RenderStateInfo, fogDensity ) );
        SetMemberName( "alpha_ref", offsetof( D3D9RenderStateInfo, alphaRef ) );
        SetMemberName( "point_size", offsetof( D3D9RenderStateInfo, pointSize ) );
        SetMemberName( "point_size_min", offsetof( D3D9RenderStateInfo, pointSizeMin ) );
        SetMemberName( "point_size_max", offsetof( D3D9RenderStateInfo, pointSizeMax ) );
        SetMemberName( "point_scale_a", offsetof( D3D9RenderStateInfo, pointScaleA ) );
        SetMemberName( "point_scale_b", offsetof( D3D9RenderStateInfo, pointScaleB ) );
        SetMemberName( "point_scale_c", offsetof( D3D9RenderStateInfo, pointScaleC ) );

        uint32_t samplerOffset = GetPushSamplerOffset( 0u );

        while ( samplerMask ) {
            uint32_t s0 = bit::tzcnt( samplerMask ); samplerMask &= samplerMask - 1u;
            uint32_t s1 = bit::tzcnt( samplerMask ); samplerMask &= samplerMask - 1u;

            std::string name = s1 < samplerCount
                ? str::format( "s", s0, "_s", s1, "_idx" )
                : str::format( "s", s0, "_idx" );

            SetMemberName( name.c_str(), samplerOffset );
            samplerOffset += sizeof( uint32_t );
        }

        return std::make_pair( rsBlock, rsMemberCount );
    }

  DxsoCompiler::DxsoCompiler(
    const std::string&        fileName,
    const DxsoModuleInfo&     moduleInfo,
    const DxsoProgramInfo&    programInfo,
    const DxsoAnalysisInfo&   analysis,
    const D3D9ConstantLayout& layout)
    : m_moduleInfo ( moduleInfo )
    , m_programInfo( programInfo )
    , m_analysis   ( &analysis )
    , m_layout     ( &layout )
    , m_module     ( spvVersion(1, 3) ) {
    // Declare an entry point ID. We'll need it during the
    // initialization phase where the execution mode is set.
    m_entryPointId = m_module.allocateId();

    // Set the shader name so that we recognize it in renderdoc
    m_module.setDebugSource(
      spv::SourceLanguageUnknown, 0,
      m_module.addDebugString(fileName.c_str()),
      nullptr);

    // Set the memory model. This is the same for all shaders.
    m_module.setMemoryModel(
      spv::AddressingModelLogical,
      spv::MemoryModelGLSL450);

    m_usedSamplers = 0;
    m_usedRTs      = 0;
    m_textureTypes = 0;
    m_rRegs.reserve(DxsoMaxTempRegs);

    for (uint32_t i = 0; i < m_rRegs.size(); i++)
      m_rRegs.at(i)  = DxsoRegisterPointer{ };

    for (uint32_t i = 0; i < m_cFloat.size(); i++)
      m_cFloat.at(i) = 0;

    for (uint32_t i = 0; i < m_cInt.size(); i++)
      m_cInt.at(i)   = 0;

    for (uint32_t i = 0; i < m_cBool.size(); i++)
      m_cBool.at(i)  = 0;

    m_vs.addr        = DxsoRegisterPointer{ };
    m_vs.oPos        = DxsoRegisterPointer{ };
    m_fog            = DxsoRegisterPointer{ };
    m_vs.oPSize      = DxsoRegisterPointer{ };

    for (uint32_t i = 0; i < m_ps.oColor.size(); i++)
      m_ps.oColor.at(i) = DxsoRegisterPointer{ };
    m_ps.oDepth      = DxsoRegisterPointer{ };
    m_ps.vFace       = DxsoRegisterPointer{ };
    m_ps.vPos        = DxsoRegisterPointer{ };

    m_loopCounter = DxsoRegisterPointer{ };

    this->emitInit();
  }


  void DxsoCompiler::processInstruction(
    const DxsoInstructionContext& ctx,
          uint32_t                currentCoissueIdx) {
    const DxsoOpcode opcode = ctx.instruction.opcode;

    for (const auto& coissue : m_analysis->coissues) {
      if (coissue.instructionIdx == ctx.instructionIdx &&
          coissue.instructionIdx != currentCoissueIdx)
        return;

      if (coissue.instructionIdx == ctx.instructionIdx + 1)
        processInstruction(coissue, coissue.instructionIdx);
    }

    switch (opcode) {
    case DxsoOpcode::Nop:
      return;

    case DxsoOpcode::Dcl:
      return this->emitDcl(ctx);

    case DxsoOpcode::Def:
    case DxsoOpcode::DefI:
    case DxsoOpcode::DefB:
      return this->emitDef(ctx);

    case DxsoOpcode::Mov:
    case DxsoOpcode::Mova:
      return this->emitMov(ctx);

    case DxsoOpcode::Add:
    case DxsoOpcode::Sub:
    case DxsoOpcode::Mad:
    case DxsoOpcode::Mul:
    case DxsoOpcode::Rcp:
    case DxsoOpcode::Rsq:
    case DxsoOpcode::Dp3:
    case DxsoOpcode::Dp4:
    case DxsoOpcode::Slt:
    case DxsoOpcode::Sge:
    case DxsoOpcode::Min:
    case DxsoOpcode::ExpP:
    case DxsoOpcode::Exp:
    case DxsoOpcode::Max:
    case DxsoOpcode::Pow:
    case DxsoOpcode::Crs:
    case DxsoOpcode::Abs:
    case DxsoOpcode::Sgn:
    case DxsoOpcode::Nrm:
    case DxsoOpcode::SinCos:
    case DxsoOpcode::Lit:
    case DxsoOpcode::Dst:
    case DxsoOpcode::LogP:
    case DxsoOpcode::Log:
    case DxsoOpcode::Lrp:
    case DxsoOpcode::Frc:
    case DxsoOpcode::Cmp:
    case DxsoOpcode::Bem:
    case DxsoOpcode::Cnd:
    case DxsoOpcode::Dp2Add:
    case DxsoOpcode::DsX:
    case DxsoOpcode::DsY:
      return this->emitVectorAlu(ctx);

    case DxsoOpcode::SetP:
      return this->emitPredicateOp(ctx);

    case DxsoOpcode::M3x2:
    case DxsoOpcode::M3x3:
    case DxsoOpcode::M3x4:
    case DxsoOpcode::M4x3:
    case DxsoOpcode::M4x4:
      return this->emitMatrixAlu(ctx);

    case DxsoOpcode::Loop:
      return this->emitControlFlowLoop(ctx);
    case DxsoOpcode::EndLoop:
      return this->emitControlFlowEndLoop(ctx);

    case DxsoOpcode::Rep:
      return this->emitControlFlowRep(ctx);
    case DxsoOpcode::EndRep:
      return this->emitControlFlowEndRep(ctx);

    case DxsoOpcode::Break:
      return this->emitControlFlowBreak(ctx);
    case DxsoOpcode::BreakC:
      return this->emitControlFlowBreakC(ctx);

    case DxsoOpcode::If:
    case DxsoOpcode::Ifc:
      return this->emitControlFlowIf(ctx);
    case DxsoOpcode::Else:
      return this->emitControlFlowElse(ctx);
    case DxsoOpcode::EndIf:
      return this->emitControlFlowEndIf(ctx);

    case DxsoOpcode::TexCoord:
      return this->emitTexCoord(ctx);

    case DxsoOpcode::Tex:
    case DxsoOpcode::TexLdl:
    case DxsoOpcode::TexLdd:
    case DxsoOpcode::TexDp3Tex:
    case DxsoOpcode::TexReg2Ar:
    case DxsoOpcode::TexReg2Gb:
    case DxsoOpcode::TexReg2Rgb:
    case DxsoOpcode::TexBem:
    case DxsoOpcode::TexBemL:
    case DxsoOpcode::TexM3x2Tex:
    case DxsoOpcode::TexM3x3Tex:
    case DxsoOpcode::TexM3x3Spec:
    case DxsoOpcode::TexM3x3VSpec:
      return this->emitTextureSample(ctx);
    case DxsoOpcode::TexKill:
      return this->emitTextureKill(ctx);
    case DxsoOpcode::TexDepth:
      return this->emitTextureDepth(ctx);

    case DxsoOpcode::TexM3x3Pad:
    case DxsoOpcode::TexM3x2Pad:
      // We don't need to do anything here, these are just padding instructions
      break;

    case DxsoOpcode::End:
    case DxsoOpcode::Comment:
    case DxsoOpcode::Phase:
      break;

    default:
      OTDU_LOG_WARN("DxsoCompiler::processInstruction: unhandled opcode: %u\n", opcode);
      break;
    }
  }

  void DxsoCompiler::finalize() {
    if (m_programInfo.type() == DxsoProgramTypes::VertexShader)
      this->emitVsFinalize();
    else
      this->emitPsFinalize();

    // Declare the entry point, we now have all the
    // information we need, including the interfaces
    m_module.addEntryPoint(m_entryPointId,
      m_programInfo.executionModel(), "main");
    m_module.setDebugName(m_entryPointId, "main");
  }


  OpenTDUOutput* DxsoCompiler::compile() {
    DxvkShaderCreateInfo info;
    info.stage = m_programInfo.shaderStage();
    info.bindingCount = m_bindings.size();
    info.bindings = m_bindings.data();
    info.inputMask = m_inputMask;
    info.outputMask = m_outputMask;
    info.sharedPushData = DxvkPushDataBlock(0u, sizeof(D3D9RenderStateInfo), 4u, 0u);
    info.localPushData = m_samplerPushData;
    info.samplerHeap = DxvkShaderBinding(VK_SHADER_STAGE_ALL, GetGlobalSamplerSetIndex(), 0u);

    if (m_programInfo.type() == DxsoProgramTypes::PixelShader)
      info.flatShadingInputs = m_ps.flatShadingMask;

    OpenTDUOutput* output = new OpenTDUOutput();
    output->Infos = info;
    output->Code = m_module.compile();
    return output;
  }

  void DxsoCompiler::emitInit() {
    // Set up common capabilities for all shaders
    m_module.enableCapability(spv::CapabilityShader);
    m_module.enableCapability(spv::CapabilityImageQuery);

    if (isSwvp()) {
      m_cFloatBuffer = this->emitDclSwvpConstantBuffer<DxsoConstantBufferType::Float>();
      m_cIntBuffer = this->emitDclSwvpConstantBuffer<DxsoConstantBufferType::Int>();
      m_cBoolBuffer = this->emitDclSwvpConstantBuffer<DxsoConstantBufferType::Bool>();
    } else {
      this->emitDclConstantBuffer();
    }

    this->emitDclInputArray();

    // Initialize the shader module with capabilities
    // etc. Each shader type has its own peculiarities.
    switch (m_programInfo.type()) {
      case DxsoProgramTypes::VertexShader: return this->emitVsInit();
      case DxsoProgramTypes::PixelShader:  return this->emitPsInit();
      default: break;
    }
  }

  void DxsoCompiler::emitDclConstantBuffer() {
    std::array<uint32_t, 2> members = {
      // int i[16 or 2048]
      m_module.defArrayTypeUnique(
        getVectorTypeId({ DxsoScalarType::Sint32, 4 }),
        m_module.constu32(m_layout->intCount)),

      // float f[256 or 224 or 8192]
      m_module.defArrayTypeUnique(
        getVectorTypeId({ DxsoScalarType::Float32, 4 }),
        m_module.constu32(m_layout->floatCount))
    };

    // Decorate array strides, this is required.
    m_module.decorateArrayStride(members[0], 16);
    m_module.decorateArrayStride(members[1], 16);

    const uint32_t structType =
      m_module.defStructType(members.size(), members.data());

    m_module.decorate(structType, false
      ? spv::DecorationBufferBlock
      : spv::DecorationBlock);

    m_module.memberDecorateOffset(structType, 0, m_layout->intOffset());
    m_module.memberDecorateOffset(structType, 1, m_layout->floatOffset());

    m_module.setDebugName(structType, "cbuffer_t");
    m_module.setDebugMemberName(structType, 0, "i");
    m_module.setDebugMemberName(structType, 1, "f");

    m_cBuffer = m_module.newVar(
      m_module.defPointerType(structType, spv::StorageClassUniform),
      spv::StorageClassUniform);

    m_module.setDebugName(m_cBuffer, "c");

    const uint32_t bindingId = computeResourceSlotId(
      m_programInfo.type(), DxsoBindingType::ConstantBuffer,
      0);

    m_module.decorateDescriptorSet(m_cBuffer, 0);
    m_module.decorateBinding(m_cBuffer, bindingId);

    auto& binding = m_bindings.emplace_back();
    binding.set             = 0u;
    binding.binding         = bindingId;
    binding.resourceIndex   = bindingId;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.access          = VK_ACCESS_UNIFORM_READ_BIT;
    binding.flags.set(DxvkDescriptorFlag::UniformBuffer);
  }

  template<DxsoConstantBufferType ConstantBufferType>
  int DxsoCompiler::emitDclSwvpConstantBuffer() {
    uint32_t member;
    bool asSsbo;
    if constexpr (ConstantBufferType == DxsoConstantBufferType::Float) {
      asSsbo = m_moduleInfo.options.vertexFloatConstantBufferAsSSBO;

      // float f[8192]
      member =  m_module.defArrayTypeUnique(
        getVectorTypeId({ DxsoScalarType::Float32, 4 }),
        m_module.constu32(m_layout->floatCount));
      m_module.decorateArrayStride(member, 16);
    } else if constexpr (ConstantBufferType == DxsoConstantBufferType::Int) {
      asSsbo = false;

      // int i[2048]
      member = m_module.defArrayTypeUnique(
          getVectorTypeId({ DxsoScalarType::Sint32, 4 }),
          m_module.constu32(m_layout->intCount));
      m_module.decorateArrayStride(member, 16);
    } else {
      asSsbo = false;

      // int i[256] (bitmasks for 2048 bools)
      member = m_module.defArrayTypeUnique(
          getVectorTypeId({ DxsoScalarType::Uint32, 4 }),
          m_module.constu32(m_layout->bitmaskCount / 4));
      // Must be a multiple of 4 otherwise.
      m_module.decorateArrayStride(member, 16);
    }

    const uint32_t structType =
      m_module.defStructType(1, &member);

    m_module.decorate(structType, asSsbo
      ? spv::DecorationBufferBlock
      : spv::DecorationBlock);

    m_module.memberDecorateOffset(structType, 0, 0);

    if constexpr (ConstantBufferType == DxsoConstantBufferType::Float) {
      m_module.setDebugName(structType, "cbuffer_float_t");
      m_module.setDebugMemberName(structType, 0, "f");
    } else if constexpr (ConstantBufferType == DxsoConstantBufferType::Int) {
      m_module.setDebugName(structType, "cbuffer_int_t");
      m_module.setDebugMemberName(structType, 0, "i");
    } else {
      m_module.setDebugName(structType, "cbuffer_bool_t");
      m_module.setDebugMemberName(structType, 0, "b");
    }

    uint32_t constantBufferId = m_module.newVar(
      m_module.defPointerType(structType, spv::StorageClassUniform),
      spv::StorageClassUniform);

    uint32_t bindingId;
    if constexpr (ConstantBufferType == DxsoConstantBufferType::Float) {
      m_module.setDebugName(constantBufferId, "cF");
      bindingId = computeResourceSlotId(
        m_programInfo.type(), DxsoBindingType::ConstantBuffer,
        0);
    } else if constexpr (ConstantBufferType == DxsoConstantBufferType::Int) {
      m_module.setDebugName(constantBufferId, "cI");
      bindingId = computeResourceSlotId(
        m_programInfo.type(), DxsoBindingType::ConstantBuffer,
        1);
    } else {
      m_module.setDebugName(constantBufferId, "cB");
      bindingId = computeResourceSlotId(
        m_programInfo.type(), DxsoBindingType::ConstantBuffer,
        2);
    }

    m_module.decorateDescriptorSet(constantBufferId, 0);
    m_module.decorateBinding(constantBufferId, bindingId);

    if (asSsbo)
      m_module.decorate(constantBufferId, spv::DecorationNonWritable);

    auto& binding = m_bindings.emplace_back();
    binding.set             = 0u;
    binding.binding         = bindingId;
    binding.resourceIndex   = bindingId;
    binding.descriptorType  = asSsbo
      ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
      : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.access          = asSsbo
      ? VK_ACCESS_SHADER_READ_BIT
      : VK_ACCESS_UNIFORM_READ_BIT;
    binding.flags.set(DxvkDescriptorFlag::UniformBuffer);

    return constantBufferId;
  }


  void DxsoCompiler::emitDclInputArray() {
    DxsoArrayType info;
    info.ctype   = DxsoScalarType::Float32;
    info.ccount  = 4;
    info.alength = DxsoMaxInterfaceRegs;

    uint32_t arrayTypeId = getArrayTypeId(info);

    // Define the actual variable. Note that this is private
    // because we will copy input registers
    // to the array during the setup phase.
    const uint32_t ptrTypeId = m_module.defPointerType(
      arrayTypeId, spv::StorageClassPrivate);

    m_vArray = m_module.newVar(
      ptrTypeId, spv::StorageClassPrivate);
    m_module.setDebugName(m_vArray, "v");
  }

  void DxsoCompiler::emitDclOutputArray() {
    DxsoArrayType info;
    info.ctype   = DxsoScalarType::Float32;
    info.ccount  = 4;
    info.alength = m_programInfo.type() == DxsoProgramTypes::VertexShader
      ? DxsoMaxInterfaceRegs
      : caps::MaxSimultaneousRenderTargets;

    uint32_t arrayTypeId = getArrayTypeId(info);

    // Define the actual variable. Note that this is private
    // because we will copy input registers
    // to the array during the setup phase.
    const uint32_t ptrTypeId = m_module.defPointerType(
      arrayTypeId, spv::StorageClassPrivate);

    m_oArray = m_module.newVar(
      ptrTypeId, spv::StorageClassPrivate);
    m_module.setDebugName(m_oArray, "o");
  }

  uint32_t _SetupSpecUBO( SpirvModule& spvModule, std::vector<DxvkBindingInfo>& bindings )
  {
      uint32_t uintType = spvModule.defIntType( 32, 0 );

      std::array<uint32_t, SpecConstantCount> specMembers;
      for ( auto& x : specMembers )
          x = uintType;

      uint32_t specStruct = spvModule.defStructTypeUnique( uint32_t( specMembers.size() ), specMembers.data() );

      spvModule.setDebugName( specStruct, "spec_state_t" );
      spvModule.decorate( specStruct, spv::DecorationBlock );

      for ( uint32_t i = 0; i < SpecConstantCount; i++ ) {
          std::string name = str::format( "dword", i );
          spvModule.setDebugMemberName( specStruct, i, name.c_str() );
          spvModule.memberDecorateOffset( specStruct, i, sizeof( uint32_t ) * i );
      }

      uint32_t specBlock = spvModule.newVar(
          spvModule.defPointerType( specStruct, spv::StorageClassUniform ),
          spv::StorageClassUniform );

      spvModule.setDebugName( specBlock, "spec_state" );
      spvModule.decorateDescriptorSet( specBlock, 0 );
      spvModule.decorateBinding( specBlock, getSpecConstantBufferSlot() );

      auto& binding = bindings.emplace_back();
      binding.set = 0u;
      binding.binding = getSpecConstantBufferSlot();
      binding.resourceIndex = getSpecConstantBufferSlot();
      binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      binding.access = VK_ACCESS_UNIFORM_READ_BIT;
      binding.flags.set( DxvkDescriptorFlag::UniformBuffer );

      return specBlock;
  }

  void DxsoCompiler::emitVsInit() {
    m_module.enableCapability(spv::CapabilityClipDistance);

    // Only VS needs this, because PS has
    // non-indexable specialized output regs
    this->emitDclOutputArray();

    // Main function of the vertex shader
    m_vs.functionId = m_module.allocateId();
    m_module.setDebugName(m_vs.functionId, "vs_main");

    this->setupRenderStateInfo(caps::MaxTexturesVS + 1u);

    m_specUbo = _SetupSpecUBO(m_module, m_bindings);

    this->emitFunctionBegin(
      m_vs.functionId,
      m_module.defVoidType(),
      m_module.defFunctionType(
        m_module.defVoidType(), 0, nullptr));
    this->emitFunctionLabel();
  }


  void DxsoCompiler::emitPsSharedConstants() {
    m_ps.sharedState = _GetSharedConstants(m_module);

    const uint32_t bindingId = computeResourceSlotId(
      m_programInfo.type(), DxsoBindingType::ConstantBuffer,
      PSShared);

    m_module.decorateDescriptorSet(m_ps.sharedState, 0);
    m_module.decorateBinding(m_ps.sharedState, bindingId);

    auto& binding = m_bindings.emplace_back();
    binding.set             = 0u;
    binding.binding         = bindingId;
    binding.resourceIndex   = bindingId;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.access          = VK_ACCESS_UNIFORM_READ_BIT;
    binding.flags.set(DxvkDescriptorFlag::UniformBuffer);
  }


  void DxsoCompiler::emitPsInit() {
    m_module.enableExtension("SPV_EXT_demote_to_helper_invocation");
    m_module.enableCapability(spv::CapabilityDemoteToHelperInvocationEXT);
    m_module.enableCapability(spv::CapabilityDerivativeControl);

    m_module.setExecutionMode(m_entryPointId,
      spv::ExecutionModeOriginUpperLeft);


    // Main function of the pixel shader
    m_ps.functionId = m_module.allocateId();
    m_module.setDebugName(m_ps.functionId, "ps_main");

    this->setupRenderStateInfo(caps::MaxTexturesPS);
    this->emitPsSharedConstants();

    m_specUbo = _SetupSpecUBO(m_module, m_bindings);

    this->emitFunctionBegin(
      m_ps.functionId,
      m_module.defVoidType(),
      m_module.defFunctionType(
        m_module.defVoidType(), 0, nullptr));
    this->emitFunctionLabel();
  }


  void DxsoCompiler::emitFunctionBegin(
          uint32_t                entryPoint,
          uint32_t                returnType,
          uint32_t                funcType) {
    this->emitFunctionEnd();

    m_module.functionBegin(
      returnType, entryPoint, funcType,
      spv::FunctionControlMaskNone);

    m_insideFunction = true;
  }


  void DxsoCompiler::emitFunctionEnd() {
    if (m_insideFunction) {
      m_module.opReturn();
      m_module.functionEnd();
    }

    m_insideFunction = false;
  }


  uint32_t DxsoCompiler::emitFunctionLabel() {
    uint32_t labelId = m_module.allocateId();
    m_module.opLabel(labelId);
    return labelId;
  }


  void DxsoCompiler::emitMainFunctionBegin() {
    this->emitFunctionBegin(
      m_entryPointId,
      m_module.defVoidType(),
      m_module.defFunctionType(
        m_module.defVoidType(), 0, nullptr));
    m_mainFuncLabel = this->emitFunctionLabel();
  }


  uint32_t DxsoCompiler::emitNewVariable(const DxsoRegisterInfo& info) {
    const uint32_t ptrTypeId = this->getPointerTypeId(info);
    return m_module.newVar(ptrTypeId, info.sclass);
  }


  uint32_t DxsoCompiler::emitNewVariableDefault(
    const DxsoRegisterInfo& info,
          uint32_t          value) {
    const uint32_t ptrTypeId = this->getPointerTypeId(info);
    if (value == 0)
      return m_module.newVar(ptrTypeId, info.sclass);
    else
      return m_module.newVarInit(ptrTypeId, info.sclass, value);
  }


  uint32_t DxsoCompiler::emitNewBuiltinVariable(
    const DxsoRegisterInfo& info,
          spv::BuiltIn      builtIn,
    const char*             name,
          uint32_t          value) {
    const uint32_t varId = emitNewVariableDefault(info, value);

    m_module.setDebugName(varId, name);
    m_module.decorateBuiltIn(varId, builtIn);

    if (m_programInfo.type() == DxsoProgramTypes::PixelShader
     && info.type.ctype != DxsoScalarType::Float32
     && info.type.ctype != DxsoScalarType::Bool
     && info.sclass == spv::StorageClassInput)
      m_module.decorate(varId, spv::DecorationFlat);

    return varId;
  }

  DxsoCfgBlock* DxsoCompiler::cfgFindBlock(
    const std::initializer_list<DxsoCfgBlockType>& types) {
    for (auto cur =  m_controlFlowBlocks.rbegin();
              cur != m_controlFlowBlocks.rend(); cur++) {
      for (auto type : types) {
        if (cur->type == type)
          return &(*cur);
      }
    }

    return nullptr;
  }

  spv::BuiltIn semanticToBuiltIn(bool input, DxsoSemantic semantic) {
    if (input)
      return spv::BuiltInMax;

    if (semantic == DxsoSemantic{ DxsoUsage::Position, 0 })
      return spv::BuiltInPosition;

    if (semantic == DxsoSemantic{ DxsoUsage::PointSize, 0 })
      return spv::BuiltInPointSize;

    return spv::BuiltInMax;
  }

  void DxsoCompiler::emitDclInterface(
            bool         input,
            uint32_t     regNumber,
            DxsoSemantic semantic,
            DxsoRegMask  mask,
            bool         centroid) {
    auto& sgn = input
      ? m_isgn : m_osgn;

    const bool pixel  = m_programInfo.type() == DxsoProgramTypes::PixelShader;
    const bool vertex = !pixel;

    if (pixel && input && semantic.usage == DxsoUsage::Color)
      centroid = true;

    uint32_t slot = 0;

    uint32_t& slots = input
      ? m_inputMask
      : m_outputMask;

    uint16_t& explicits = input
      ? m_explicitInputs
      : m_explicitOutputs;

    // Some things we consider builtins could be packed in an output reg.
    bool builtin = semanticToBuiltIn(input, semantic) != spv::BuiltInMax;

    uint32_t i = sgn.elemCount++;

    if (input && vertex) {
      // Any slot will do! Let's chose the next one
      slot = i;
    }
    else if ( (!input && vertex)
           || (input  && pixel ) ) {
      // Don't register the slot if it belongs to a builtin
      if (!builtin)
        slot = RegisterLinkerSlot(semantic);
    }
    else { //if (!input && pixel)
      // We want to make the output slot the same as the
      // output register for pixel shaders so they go to
      // the right render target.
      slot = regNumber;
    }

    // Don't want to mark down any of these builtins.
    if (!builtin)
      slots   |= 1u << slot;
    explicits |= 1u << regNumber;

    auto& elem = sgn.elems[i];
    elem.slot      = slot;
    elem.regNumber = regNumber;
    elem.semantic  = semantic;
    elem.mask      = mask;
    elem.centroid  = centroid;
  }

  void DxsoCompiler::emitDclSampler(
          uint32_t        idx,
          DxsoTextureType type) {
    m_usedSamplers |= (1u << idx);

    if (!m_samplerArray)
      m_samplerArray = _SetupSamplerArray(m_module);

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

    auto DclSampler = [this, &viewType](
      uint32_t        idx,
      uint32_t        bindingId,
      DxsoSamplerType type,
      bool            depth,
      bool            implicit) {
      // Setup our combines sampler.
      DxsoSamplerInfo& sampler = !depth
        ? m_samplers[idx].color[type]
        : m_samplers[idx].depth[type];

      spv::Dim dimensionality;

      const char* suffix = "_2d";

      switch (type) {
        default:
        case SamplerTypeTexture2D:
          sampler.dimensions = 2;
          dimensionality = spv::Dim2D;
          viewType = VK_IMAGE_VIEW_TYPE_2D;
          break;

        case SamplerTypeTextureCube:
          suffix = "_cube";
          sampler.dimensions = 3;
          dimensionality = spv::DimCube;
          viewType = VK_IMAGE_VIEW_TYPE_CUBE;
          break;

        case SamplerTypeTexture3D:
          suffix = "_3d";
          sampler.dimensions = 3;
          dimensionality = spv::Dim3D;
          viewType = VK_IMAGE_VIEW_TYPE_3D;
          break;
      }

      sampler.imageTypeId = m_module.defImageType(
        m_module.defFloatType(32),
        dimensionality, depth ? 1 : 0, 0, 0, 1,
        spv::ImageFormatUnknown);

      sampler.imageVarId = m_module.newVar(
        m_module.defPointerType(
          sampler.imageTypeId, spv::StorageClassUniformConstant),
        spv::StorageClassUniformConstant);

      sampler.sampledTypeId = m_module.defSampledImageType(sampler.imageTypeId);
      sampler.samplerIndex = idx;

      std::string name = str::format("t", idx, suffix, depth ? "_shadow" : "");
      m_module.setDebugName(sampler.imageVarId, name.c_str());

      m_module.decorateDescriptorSet(sampler.imageVarId, 0);
      m_module.decorateBinding      (sampler.imageVarId, bindingId);
    };

    const uint32_t binding = computeResourceSlotId(m_programInfo.type(),
      DxsoBindingType::Image,
      idx);

    const bool implicit = m_programInfo.majorVersion() < 2 || m_moduleInfo.options.forceSamplerTypeSpecConstants;

    if (!implicit) {
      DxsoSamplerType samplerType = 
        SamplerTypeFromTextureType(type);

      DclSampler(idx, binding, samplerType, false, implicit);

      if (samplerType != SamplerTypeTexture3D) {
        // We could also be depth compared!
        DclSampler(idx, binding, samplerType, true, implicit);
      }

      const uint32_t offset = idx * 2;
      uint32_t textureBits = uint32_t(viewType);
      m_textureTypes |= textureBits << offset;
    }
    else {
      // Could be any of these!
      // We will check with the spec constant at sample time.
      for (uint32_t i = 0; i < SamplerTypeCount; i++) {
        auto samplerType = static_cast<DxsoSamplerType>(i);

        DclSampler(idx, binding, samplerType, false, implicit);

        if (samplerType != SamplerTypeTexture3D)
          DclSampler(idx, binding, samplerType, true, implicit);
      }
    }

    m_samplers[idx].type = type;

    auto& imageBinding = m_bindings.emplace_back();
    imageBinding.set             = 0u;
    imageBinding.binding         = binding;
    imageBinding.resourceIndex   = binding;
    imageBinding.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    imageBinding.viewType        = implicit ? VK_IMAGE_VIEW_TYPE_MAX_ENUM : viewType;
    imageBinding.access          = VK_ACCESS_SHADER_READ_BIT;

    auto& samplerBinding = m_bindings.emplace_back();
    samplerBinding.resourceIndex  = binding;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBinding.blockOffset    = GetPushSamplerOffset(idx);
    samplerBinding.flags.set(DxvkDescriptorFlag::PushData);
  }


  uint32_t DxsoCompiler::emitArrayIndex(
            uint32_t          idx,
      const DxsoBaseRegister* relative) {
    uint32_t result = m_module.consti32(idx);

    if (relative != nullptr) {
      DxsoRegisterValue offset = emitRegisterLoad(*relative, DxsoRegMask(true, false, false, false), nullptr);

      result = m_module.opIAdd(
        getVectorTypeId(offset.type),
        result, offset.id);
    }

    return result;
  }


  DxsoRegisterPointer DxsoCompiler::emitInputPtr(
            bool              texture,
      const DxsoBaseRegister& reg,
      const DxsoBaseRegister* relative) {
    uint32_t idx = reg.id.num;

    // Account for the two color regs.
    if (texture)
      idx += 2;

    DxsoRegisterPointer input;

    input.type = DxsoVectorType{ DxsoScalarType::Float32, 4 };

    uint32_t index = this->emitArrayIndex(idx, relative);

    const uint32_t typeId = getVectorTypeId(input.type);
    input.id = m_module.opAccessChain(
      m_module.defPointerType(typeId, spv::StorageClassPrivate),
      m_vArray,
      1, &index);

    return input;
  }

  DxsoRegisterPointer DxsoCompiler::emitRegisterPtr(
      const char*             name,
            DxsoScalarType    ctype,
            uint32_t          ccount,
            uint32_t          defaultVal,
            spv::StorageClass storageClass,
            spv::BuiltIn      builtIn) {
    DxsoRegisterPointer result;

    DxsoRegisterInfo info;
    info.type.ctype    = ctype;
    info.type.ccount   = ccount;
    info.type.alength  = 1;
    info.sclass        = storageClass;

    result.type = DxsoVectorType{ ctype, ccount };
    if (builtIn == spv::BuiltInMax) {
      result.id = this->emitNewVariableDefault(info, defaultVal);
      m_module.setDebugName(result.id, name);
    }
    else {
      result.id = this->emitNewBuiltinVariable(
        info, builtIn, name, defaultVal);
    }

    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitLoadConstant(
      const DxsoBaseRegister& reg,
      const DxsoBaseRegister* relative) {

    // SWVP cbuffers:           Member    Binding index
    // float                    f[8192];  0
    // int32_t                  i[2048];  1
    // bool (uint32_t bitmask)  i[[256]]; 2

    // HWVP cbuffer:            Member         Member index
    // int32_t                  i[16];         0
    // float                    f[256 or 224]; 1
    //
    // bools as spec constant bitmasks
    DxsoRegisterValue result = { };

    switch (reg.id.type) {
      case DxsoRegisterType::Const:
        result.type = { DxsoScalarType::Float32, 4 };

        if (!relative)
          result.id = m_cFloat.at(reg.id.num);
        break;
      
      case DxsoRegisterType::ConstInt:
        result.type = { DxsoScalarType::Sint32, 4 };
        result.id = m_cInt.at(reg.id.num);
        break;
      
      case DxsoRegisterType::ConstBool:
        result.type = { DxsoScalarType::Bool, 1 };
        result.id = m_cBool.at(reg.id.num);
        break;
      
      default: break;
    }

    if (result.id)
      return result;

    switch (reg.id.type) {
      case DxsoRegisterType::Const:
        if (!relative) {
          m_meta.maxConstIndexF = std::max(m_meta.maxConstIndexF, reg.id.num + 1);
          m_meta.maxConstIndexF = std::min(m_meta.maxConstIndexF, m_layout->floatCount);
        } else {
          m_meta.maxConstIndexF = m_layout->floatCount;
          m_meta.needsConstantCopies |= m_moduleInfo.options.strictConstantCopies
                                     || m_cFloat.at(reg.id.num) != 0;
        }
        break;
      
      case DxsoRegisterType::ConstInt:
        m_meta.maxConstIndexI = std::max(m_meta.maxConstIndexI, reg.id.num + 1);
        m_meta.maxConstIndexI = std::min(m_meta.maxConstIndexI, m_layout->intCount);
        break;
      
      case DxsoRegisterType::ConstBool:
        m_meta.maxConstIndexB = std::max(m_meta.maxConstIndexB, reg.id.num + 1);
        m_meta.maxConstIndexB = std::min(m_meta.maxConstIndexB, m_layout->boolCount);
        m_meta.boolConstantMask |= 1 << reg.id.num;
        break;
      
      default: break;
    }

    uint32_t relativeIdx = this->emitArrayIndex(reg.id.num, relative);

    if (reg.id.type != DxsoRegisterType::ConstBool) {
      uint32_t structIdx;
      uint32_t cBufferId;

      if (reg.id.type == DxsoRegisterType::Const) {
        if (isSwvp()) {
          structIdx = m_module.constu32(0);
          cBufferId = m_cFloatBuffer;
        } else {
          structIdx = m_module.constu32(1);
          cBufferId = m_cBuffer;
        }
      } else {
        if (isSwvp()) {
          structIdx = m_module.constu32(0);
          cBufferId = m_cIntBuffer;
        } else {
          structIdx = m_module.constu32(0);
          cBufferId = m_cBuffer;
        }
      }

      std::array<uint32_t, 2> indices = { structIdx, relativeIdx };

      uint32_t typeId = getVectorTypeId(result.type);
      uint32_t ptrId = m_module.opAccessChain(
        m_module.defPointerType(typeId, spv::StorageClassUniform),
        cBufferId, indices.size(), indices.data());

      result.id = m_module.opLoad(typeId, ptrId);

      if (relative && !m_moduleInfo.options.robustness2Supported) {
        uint32_t constCount = m_module.constu32(m_layout->floatCount);

        // Expand condition to bvec4 since the result has four components
        uint32_t cond = m_module.opULessThan(m_module.defBoolType(), relativeIdx, constCount);
        std::array<uint32_t, 4> condIds = { cond, cond, cond, cond };

        cond = m_module.opCompositeConstruct(
          m_module.defVectorType(m_module.defBoolType(), 4),
          condIds.size(), condIds.data());

        result.id = m_module.opSelect(typeId, cond, result.id,
          m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f));
      }
    } else {
      // Bool constants have no relative indexing, so we can do the bitfield
      // magic for SWVP at compile time.

      uint32_t uintType  = getScalarTypeId(DxsoScalarType::Uint32);
      uint32_t uvec4Type = getVectorTypeId({ DxsoScalarType::Uint32, 4 });

      // If not SWVP, spec const this
      uint32_t bit;
      if (m_layout->bitmaskCount != 1) {
        std::array<uint32_t, 2> indices = { m_module.constu32(0), m_module.constu32(reg.id.num / 128) };

        uint32_t indexCount = m_layout->bitmaskCount == 1 ? 1 : 2;
        uint32_t accessType = m_layout->bitmaskCount == 1 ? uintType : uvec4Type;

        uint32_t ptrId = m_module.opAccessChain(
          m_module.defPointerType(accessType, spv::StorageClassUniform),
          m_cBoolBuffer, indexCount, indices.data());

        uint32_t bitfield = m_module.opLoad(accessType, ptrId);
        uint32_t bitIdx = m_module.consti32(reg.id.num % 32);

        uint32_t index = (reg.id.num % 128) / 32;
        bitfield = m_module.opCompositeExtract(uintType, bitfield, 1, &index);

        bit = m_module.opBitFieldUExtract(
          uintType, bitfield, bitIdx, m_module.consti32(1));
      }
      else {
        bit = m_spec.get(m_module, m_specUbo,
          m_programInfo.type() == DxsoProgramType::VertexShader
            ? SpecVertexShaderBools
            : SpecPixelShaderBools,
          reg.id.num, 1);
      }

      result.id = m_module.opINotEqual(
        getVectorTypeId(result.type),
        bit, m_module.constu32(0));
    }

    return result;
  }


  DxsoRegisterPointer DxsoCompiler::emitOutputPtr(
            bool              texcrdOut,
      const DxsoBaseRegister& reg,
      const DxsoBaseRegister* relative) {
    uint32_t idx = reg.id.num;

    // Account for the two color regs.
    if (texcrdOut)
      idx += 2;

    DxsoRegisterPointer input;

    input.type = DxsoVectorType{ DxsoScalarType::Float32, 4 };

    uint32_t index = this->emitArrayIndex(idx, relative);

    const uint32_t typeId = getVectorTypeId(input.type);
    input.id = m_module.opAccessChain(
      m_module.defPointerType(typeId, spv::StorageClassPrivate),
      m_oArray,
      1, &index);

    return input;
  }


  DxsoRegisterPointer DxsoCompiler::emitGetOperandPtr(
      const DxsoBaseRegister& reg,
      const DxsoBaseRegister* relative) {
    switch (reg.id.type) {
      case DxsoRegisterType::Temp: {
        if (reg.id.num >= m_rRegs.size())
          m_rRegs.resize( reg.id.num + 1, DxsoRegisterPointer { } );
        DxsoRegisterPointer& ptr = m_rRegs.at(reg.id.num);
        if (ptr.id == 0) {
          std::string name = str::format("r", reg.id.num);
          ptr = this->emitRegisterPtr(
            name.c_str(), DxsoScalarType::Float32, 4,
            m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f));
        }
        return ptr;
      }

      case DxsoRegisterType::Input: {
        if (!(m_explicitInputs & 1u << reg.id.num)) {
          this->emitDclInterface(
            true, reg.id.num,
            DxsoSemantic{ DxsoUsage::Color, reg.id.num },
            IdentityWriteMask, false);
        }

        return this->emitInputPtr(false, reg, relative);
      }

      case DxsoRegisterType::PixelTexcoord:
      case DxsoRegisterType::Texture: {
        if (m_programInfo.type() == DxsoProgramTypes::PixelShader) {
          // Texture register

          // SM2, or SM 1.4
          if (reg.id.type == DxsoRegisterType::PixelTexcoord
          ||  m_programInfo.majorVersion() >= 2
          || (m_programInfo.majorVersion() == 1
           && m_programInfo.minorVersion() == 4)) {
            uint32_t adjustedNumber = reg.id.num + 2;
            if (!(m_explicitInputs & 1u << adjustedNumber)) {
              this->emitDclInterface(
                true, adjustedNumber,
                DxsoSemantic{ DxsoUsage::Texcoord, reg.id.num },
                IdentityWriteMask, false);
            }

            return this->emitInputPtr(true, reg, relative);
          }
          else {
            // User must use tex/texcoord to put data in this private register.
            // We use the an oob id which fxc never generates for the texcoord data.
            DxsoRegisterPointer& ptr = m_tRegs.at(reg.id.num);
            if (ptr.id == 0) {
              std::string name = str::format("t", reg.id.num);
              ptr = this->emitRegisterPtr(
                name.c_str(), DxsoScalarType::Float32, 4,
                m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f));
            }
            return ptr;
          }
        }
        else {
          // Address register
          if (m_vs.addr.id == 0) {
            m_vs.addr = this->emitRegisterPtr(
              "a0", DxsoScalarType::Sint32, 4,
              m_module.constvec4i32(0, 0, 0, 0));
          }
          return m_vs.addr;
        }
      }

      case DxsoRegisterType::RasterizerOut:
        switch (reg.id.num) {
          case RasterOutPosition:
            if (m_vs.oPos.id == 0) {
              m_vs.oPos = this->emitRegisterPtr(
                "oPos", DxsoScalarType::Float32, 4,
                m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f),
                spv::StorageClassOutput, spv::BuiltInPosition);
            }
            return m_vs.oPos;

          case RasterOutFog:
            if (m_fog.id == 0) {
              bool input = m_programInfo.type() == DxsoProgramType::PixelShader;
              DxsoSemantic semantic = DxsoSemantic{ DxsoUsage::Fog, 0 };

              uint32_t slot = RegisterLinkerSlot(semantic);

              uint32_t& slots = input
                ? m_inputMask
                : m_outputMask;

              slots |= 1u << slot;

              m_fog = this->emitRegisterPtr(
                input ? "vFog" : "oFog",
                DxsoScalarType::Float32, 1,
                input ? 0 : m_module.constf32(1.0f),
                input ? spv::StorageClassInput : spv::StorageClassOutput);

              m_module.decorateLocation(m_fog.id, slot);
            }
            return m_fog;

          case RasterOutPointSize:
            if (m_vs.oPSize.id == 0) {
              m_vs.oPSize = this->emitRegisterPtr(
                "oPSize", DxsoScalarType::Float32, 1,
                m_module.constf32(0.0f),
                spv::StorageClassOutput, spv::BuiltInPointSize);
            }
            return m_vs.oPSize;

          default: {
            DxsoRegisterPointer nullPointer = { };
            return nullPointer;
          }
        }

      case DxsoRegisterType::ColorOut: {
        uint32_t idx = std::min(reg.id.num, 4u);

        if (m_ps.oColor[idx].id == 0) {
          std::string name = str::format("oC", idx);
          m_ps.oColor[idx] = this->emitRegisterPtr(
            name.c_str(), DxsoScalarType::Float32, 4,
            m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f),
            spv::StorageClassOutput);

          m_outputMask |= 1u << idx;
          m_module.decorateLocation(m_ps.oColor[idx].id, idx);
          m_module.decorateIndex(m_ps.oColor[idx].id, 0);

          m_usedRTs |= (1u << idx);
        }
        return m_ps.oColor[idx];
      }

      case DxsoRegisterType::AttributeOut: {
        auto ptr = this->emitOutputPtr(false, reg, nullptr);

        if (!(m_explicitOutputs & 1u << reg.id.num)) {
          this->emitDclInterface(
            false, reg.id.num,
            DxsoSemantic{ DxsoUsage::Color, reg.id.num },
            IdentityWriteMask, false);

          m_module.opStore(ptr.id, m_module.constfReplicant(0, ptr.type.ccount));
        }

        return ptr;
      }

      case DxsoRegisterType::Output: {
        bool texcrdOut = m_programInfo.type() == DxsoProgramTypes::VertexShader
                      && m_programInfo.majorVersion() != 3;

        auto ptr = this->emitOutputPtr(texcrdOut, reg, !texcrdOut ? relative : nullptr);

        if (texcrdOut) {
          uint32_t adjustedNumber = reg.id.num + 2;
          if (!(m_explicitOutputs & 1u << adjustedNumber)) {
            this->emitDclInterface(
              false, adjustedNumber,
              DxsoSemantic{ DxsoUsage::Texcoord, reg.id.num },
              IdentityWriteMask, false);

            m_module.opStore(ptr.id, m_module.constfReplicant(0, ptr.type.ccount));
          }
        }

        return ptr;
      }

      case DxsoRegisterType::DepthOut:
        if (m_ps.oDepth.id == 0) {
          m_module.setExecutionMode(m_entryPointId,
            spv::ExecutionModeDepthReplacing);

          m_ps.oDepth = this->emitRegisterPtr(
            "oDepth", DxsoScalarType::Float32, 1,
            m_module.constf32(0.0f),
            spv::StorageClassOutput, spv::BuiltInFragDepth);
        }
        return m_ps.oDepth;

      case DxsoRegisterType::Loop:
        if (m_loopCounter.id == 0) {
          m_loopCounter = this->emitRegisterPtr(
            "aL", DxsoScalarType::Sint32, 1,
            m_module.consti32(0));
        }
        return m_loopCounter;

      case DxsoRegisterType::MiscType:
        if (reg.id.num == MiscTypePosition) {
          if (m_ps.vPos.id == 0) {
            m_ps.vPos = this->emitRegisterPtr(
              "vPos", DxsoScalarType::Float32, 4, 0);
          }
          return m_ps.vPos;
        }
        else { // MiscTypeFace
          if (m_ps.vFace.id == 0) {
            m_ps.vFace = this->emitRegisterPtr(
              "vFace", DxsoScalarType::Float32, 4, 0);
          }
          return m_ps.vFace;
        }

      case DxsoRegisterType::Predicate: {
        DxsoRegisterPointer& ptr = m_pRegs.at(reg.id.num);
        if (ptr.id == 0) {
          std::string name = str::format("p", reg.id.num);
          ptr = this->emitRegisterPtr(
            name.c_str(), DxsoScalarType::Bool, 4,
            m_module.constvec4b32(false, false, false, false));
        }
        return ptr;
      }

      default: {
        //Logger::warn(str::format("emitGetOperandPtr: unhandled reg type: ", reg.id.type));

        DxsoRegisterPointer nullPointer = { };
        return nullPointer;
      }
    }
  }


  uint32_t DxsoCompiler::emitBoolComparison(DxsoVectorType type, DxsoComparison cmp, uint32_t a, uint32_t b) {
    const uint32_t typeId = getVectorTypeId(type);
    switch (cmp) {
      default:
      case DxsoComparison::Never:        return m_module.constbReplicant(false, type.ccount);  break;
      case DxsoComparison::GreaterThan:  return m_module.opFOrdGreaterThan     (typeId, a, b); break;
      case DxsoComparison::Equal:        return m_module.opFOrdEqual           (typeId, a, b); break;
      case DxsoComparison::GreaterEqual: return m_module.opFOrdGreaterThanEqual(typeId, a, b); break;
      case DxsoComparison::LessThan:     return m_module.opFOrdLessThan        (typeId, a, b); break;
      case DxsoComparison::NotEqual:     return m_module.opFUnordNotEqual      (typeId, a, b); break;
      case DxsoComparison::LessEqual:    return m_module.opFOrdLessThanEqual   (typeId, a, b); break;
      case DxsoComparison::Always:       return m_module.constbReplicant(true, type.ccount);   break;
    }
  }


  DxsoRegisterValue DxsoCompiler::emitValueLoad(
            DxsoRegisterPointer ptr) {
    DxsoRegisterValue result;
    result.type = ptr.type;
    result.id   = m_module.opLoad(
      getVectorTypeId(result.type),
      ptr.id);
    return result;
  }


  DxsoRegisterValue DxsoCompiler::applyPredicate(DxsoRegisterValue pred, DxsoRegisterValue dst, DxsoRegisterValue src) {
    if (dst.type.ccount != pred.type.ccount) {
      DxsoRegMask mask = DxsoRegMask(
        pred.type.ccount > 0,
        pred.type.ccount > 1,
        pred.type.ccount > 2,
        pred.type.ccount > 3);

      pred = emitRegisterSwizzle(pred, IdentitySwizzle, mask);
    }

    dst.id = m_module.opSelect(
      getVectorTypeId(dst.type),
      pred.id,
      src.id, dst.id);

    return dst;
  }


  void DxsoCompiler::emitValueStore(
          DxsoRegisterPointer     ptr,
          DxsoRegisterValue       value,
          DxsoRegMask             writeMask,
          DxsoRegisterValue       predicate) {
    // If the source value consists of only one component,
    // it is stored in all components of the destination.
    if (value.type.ccount == 1)
      value = emitRegisterExtend(value, writeMask.popCount());

    if (ptr.type.ccount == writeMask.popCount()) {
      if (predicate.id)
        value = applyPredicate(predicate, emitValueLoad(ptr), value);

      // Simple case: We write to the entire register
      m_module.opStore(ptr.id, value.id);
    } else {
      // We only write to part of the destination
      // register, so we need to load and modify it
      DxsoRegisterValue tmp = emitValueLoad(ptr);
      tmp = emitRegisterInsert(tmp, value, writeMask);

      if (predicate.id)
        value = applyPredicate(predicate, emitValueLoad(ptr), tmp);

      m_module.opStore(ptr.id, tmp.id);
    }
  }


  DxsoRegisterValue DxsoCompiler::emitClampBoundReplicant(
            DxsoRegisterValue       srcValue,
            float                   lb,
            float                   ub) {
    srcValue.id = m_module.opFClamp(getVectorTypeId(srcValue.type), srcValue.id,
      m_module.constfReplicant(lb, srcValue.type.ccount),
      m_module.constfReplicant(ub, srcValue.type.ccount));

    return srcValue;
  }


  DxsoRegisterValue DxsoCompiler::emitSaturate(
            DxsoRegisterValue       srcValue) {
    return emitClampBoundReplicant(srcValue, 0.0f, 1.0f);
  }


  DxsoRegisterValue DxsoCompiler::emitMulOperand(
          DxsoRegisterValue       operand,
          DxsoRegisterValue       other) {
    if (m_moduleInfo.options.d3d9FloatEmulation != D3D9FloatEmulation::Strict || operand.id == other.id)
      return operand;

    uint32_t boolId = getVectorTypeId({ DxsoScalarType::Bool, other.type.ccount });
    uint32_t zeroId = m_module.constfReplicant(0.0f, other.type.ccount);

    DxsoRegisterValue result;
    result.type = operand.type;
    result.id = m_module.opSelect(getVectorTypeId(result.type),
      m_module.opFOrdEqual(boolId, other.id, zeroId), zeroId, operand.id);
    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitMul(
          DxsoRegisterValue       a,
          DxsoRegisterValue       b) {
    auto az = emitMulOperand(a, b);
    auto bz = emitMulOperand(b, a);

    DxsoRegisterValue result;
    result.type = a.type;
    result.id = m_module.opFMul(getVectorTypeId(result.type), az.id, bz.id);
    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitMad(
          DxsoRegisterValue       a,
          DxsoRegisterValue       b,
          DxsoRegisterValue       c) {
    DxsoRegisterValue result = emitMul(a, b);
    result.id = m_module.opFAdd(getVectorTypeId(result.type), result.id, c.id);
    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitDot(
            DxsoRegisterValue       a,
            DxsoRegisterValue       b) {
    auto az = emitMulOperand(a, b);
    auto bz = emitMulOperand(b, a);

    DxsoRegisterValue dot;
    dot.type.ctype  = a.type.ctype;
    dot.type.ccount = 1;
    dot.id          = 0;

    uint32_t componentType = getVectorTypeId(dot.type);

    for (uint32_t i = 0; i < a.type.ccount; i++) {
      uint32_t product = m_module.opFMul(componentType,
        m_module.opCompositeExtract(componentType, az.id, 1, &i),
        m_module.opCompositeExtract(componentType, bz.id, 1, &i));

      dot.id = dot.id ? m_module.opFAdd(componentType, dot.id, product) : product;
    }

    return dot;
  }

  DxsoRegisterValue DxsoCompiler::emitMix(
            DxsoRegisterValue       x,
            DxsoRegisterValue       y,
            DxsoRegisterValue       a) {
    uint32_t typeId = getVectorTypeId(x.type);

    DxsoRegisterValue ySubx;
    ySubx.type = x.type;
    ySubx.id   = m_module.opFSub(typeId, y.id, x.id);

    return emitMad(a, ySubx, x);
  }


  DxsoRegisterValue DxsoCompiler::emitCross(
          DxsoRegisterValue       a,
          DxsoRegisterValue       b) {
    uint32_t typeId = getVectorTypeId(a.type);

    const std::array<uint32_t, 4> shiftIndices = { 1, 2, 0, 1 };

    DxsoRegisterValue result;
    result.type = { DxsoScalarType::Float32, 3 };

    std::array<DxsoRegisterValue, 2> products;

    for (uint32_t i = 0; i < 2; i++) {
      DxsoRegisterValue ashift;
      ashift.type = result.type;
      ashift.id = m_module.opVectorShuffle(typeId,
        a.id, a.id, 3, &shiftIndices[i]);

      DxsoRegisterValue bshift;
      bshift.type = result.type;
      bshift.id = m_module.opVectorShuffle(typeId,
        b.id, b.id, 3, &shiftIndices[1 - i]);

      products[i] = emitMul(ashift, bshift);
    }

    result.id = m_module.opFSub(typeId, products[0].id, products[1].id);
    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitRegisterInsert(
            DxsoRegisterValue       dstValue,
            DxsoRegisterValue       srcValue,
            DxsoRegMask             srcMask) {
    DxsoRegisterValue result;
    result.type = dstValue.type;

    const uint32_t typeId = getVectorTypeId(result.type);

    if (srcMask.popCount() == 0) {
      // Nothing to do if the insertion mask is empty
      result.id = dstValue.id;
    } else if (dstValue.type.ccount == 1) {
      // Both values are scalar, so the first component
      // of the write mask decides which one to take.
      result.id = srcMask[0] ? srcValue.id : dstValue.id;
    } else if (srcValue.type.ccount == 1) {
      // The source value is scalar. Since OpVectorShuffle
      // requires both arguments to be vectors, we have to
      // use OpCompositeInsert to modify the vector instead.
      const uint32_t componentId = srcMask.firstSet();

      result.id = m_module.opCompositeInsert(typeId,
        srcValue.id, dstValue.id, 1, &componentId);
    } else {
      // Both arguments are vectors. We can determine which
      // components to take from which vector and use the
      // OpVectorShuffle instruction.
      std::array<uint32_t, 4> components;
      uint32_t srcComponentId = dstValue.type.ccount;

      for (uint32_t i = 0; i < dstValue.type.ccount; i++)
        components.at(i) = srcMask[i] ? srcComponentId++ : i;

      result.id = m_module.opVectorShuffle(
        typeId, dstValue.id, srcValue.id,
        dstValue.type.ccount, components.data());
    }

    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitRegisterLoadRaw(
      const DxsoBaseRegister& reg,
      const DxsoBaseRegister* relative) {
    switch (reg.id.type) {
      case DxsoRegisterType::Const:
      case DxsoRegisterType::ConstInt:
      case DxsoRegisterType::ConstBool:
        return emitLoadConstant(reg, relative);
      
      default:
        return emitValueLoad(emitGetOperandPtr(reg, relative));
    }
  }


  DxsoRegisterValue DxsoCompiler::emitRegisterExtend(
            DxsoRegisterValue       value,
            uint32_t                size) {
    if (size == 1)
      return value;

    std::array<uint32_t, 4> ids = {{
      value.id, value.id,
      value.id, value.id,
    }};

    DxsoRegisterValue result;
    result.type.ctype  = value.type.ctype;
    result.type.ccount = size;
    result.id = m_module.opCompositeConstruct(
      getVectorTypeId(result.type),
      size, ids.data());
    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitRegisterSwizzle(
            DxsoRegisterValue       value,
            DxsoRegSwizzle          swizzle,
            DxsoRegMask             writeMask) {
    if (value.type.ccount == 1)
      return emitRegisterExtend(value, writeMask.popCount());

    std::array<uint32_t, 4> indices;

    uint32_t dstIndex = 0;

    for (uint32_t i = 0; i < 4; i++) {
      if (writeMask[i])
        indices[dstIndex++] = swizzle[i];
    }

    // If the swizzle combined with the mask can be reduced
    // to a no-op, we don't need to insert any instructions.
    bool isIdentitySwizzle = dstIndex == value.type.ccount;

    for (uint32_t i = 0; i < dstIndex && isIdentitySwizzle; i++)
      isIdentitySwizzle &= indices[i] == i;

    if (isIdentitySwizzle)
      return value;

    // Use OpCompositeExtract if the resulting vector contains
    // only one component, and OpVectorShuffle if it is a vector.
    DxsoRegisterValue result;
    result.type.ctype  = value.type.ctype;
    result.type.ccount = dstIndex;

    const uint32_t typeId = getVectorTypeId(result.type);

    if (dstIndex == 1) {
      result.id = m_module.opCompositeExtract(
        typeId, value.id, 1, indices.data());
    } else {
      result.id = m_module.opVectorShuffle(
        typeId, value.id, value.id,
        dstIndex, indices.data());
    }

    return result;
  }


  DxsoRegisterValue DxsoCompiler::emitSrcOperandPreSwizzleModifiers(
            DxsoRegisterValue       value,
            DxsoRegModifier         modifier) {
    // r / r.z
    // r / r.w
    if (modifier == DxsoRegModifier::Dz
     || modifier == DxsoRegModifier::Dw) {
      const uint32_t index = modifier == DxsoRegModifier::Dz ? 2 : 3;

      std::array<uint32_t, 4> indices = { index, index, index, index };

      uint32_t component = m_module.opVectorShuffle(
        getVectorTypeId(value.type), value.id, value.id, value.type.ccount, indices.data());

      value.id = m_module.opFDiv(
        getVectorTypeId(value.type), value.id, component);
    }

    return value;
  }


  DxsoRegisterValue DxsoCompiler::emitSrcOperandPostSwizzleModifiers(
            DxsoRegisterValue       value,
            DxsoRegModifier         modifier) {
    // r - 0.5
    if (modifier == DxsoRegModifier::Bias
     || modifier == DxsoRegModifier::BiasNeg) {
      uint32_t halfVec = m_module.constfReplicant(
        0.5f, value.type.ccount);

      value.id = m_module.opFSub(
        getVectorTypeId(value.type), value.id, halfVec);
    }

    // fma(r, 2.0f, -1.0f)
    if (modifier == DxsoRegModifier::Sign
     || modifier == DxsoRegModifier::SignNeg) {
      uint32_t twoVec = m_module.constfReplicant(
        2.0f, value.type.ccount);

      uint32_t minusOneVec = m_module.constfReplicant(
        -1.0f, value.type.ccount);

      value.id = m_module.opFFma(
        getVectorTypeId(value.type), value.id, twoVec, minusOneVec);
    }

    // 1 - r
    if (modifier == DxsoRegModifier::Comp) {
      uint32_t oneVec = m_module.constfReplicant(
        1.0f, value.type.ccount);

      value.id = m_module.opFSub(
        getVectorTypeId(value.type), oneVec, value.id);
    }

    // r * 2
    if (modifier == DxsoRegModifier::X2
     || modifier == DxsoRegModifier::X2Neg) {
      uint32_t twoVec = m_module.constfReplicant(
        2.0f, value.type.ccount);

      value.id = m_module.opFMul(
        getVectorTypeId(value.type), value.id, twoVec);
    }

    // abs( r )
    if (modifier == DxsoRegModifier::Abs
     || modifier == DxsoRegModifier::AbsNeg) {
      value.id = m_module.opFAbs(
        getVectorTypeId(value.type), value.id);
    }

    // !r
    if (modifier == DxsoRegModifier::Not) {
      value.id =
        m_module.opLogicalNot(getVectorTypeId(value.type), value.id);
    }

    // -r
    // Treating as -r
    // Treating as -r
    // -r * 2
    // -abs(r)
    if (modifier == DxsoRegModifier::Neg
     || modifier == DxsoRegModifier::BiasNeg
     || modifier == DxsoRegModifier::SignNeg
     || modifier == DxsoRegModifier::X2Neg
     || modifier == DxsoRegModifier::AbsNeg) {
      value.id = m_module.opFNegate(
        getVectorTypeId(value.type), value.id);
    }

    return value;
  }

  DxsoRegisterValue DxsoCompiler::emitRegisterLoad(
      const DxsoBaseRegister& reg,
            DxsoRegMask       writeMask,
      const DxsoBaseRegister* relative) {
    // Load operand from the operand pointer
    DxsoRegisterValue result = emitRegisterLoadRaw(reg, relative);

    // PS 1.x clamps float constants
    if (m_programInfo.type() == DxsoProgramType::PixelShader && m_programInfo.majorVersion() == 1
      && reg.id.type == DxsoRegisterType::Const)
      result = emitClampBoundReplicant(result, -1.0f, 1.0f);

    // Apply operand modifiers
    result = emitSrcOperandPreSwizzleModifiers(result, reg.modifier);

    // Apply operand swizzle to the operand value
    result = emitRegisterSwizzle(result, reg.swizzle, writeMask);

    // Apply operand modifiers
    result = emitSrcOperandPostSwizzleModifiers(result, reg.modifier);
    return result;
  }

  void DxsoCompiler::emitDcl(const DxsoInstructionContext& ctx) {
    auto id = ctx.dst.id;

    if (id.type == DxsoRegisterType::Sampler) {
      this->emitDclSampler(
        ctx.dst.id.num,
        ctx.dcl.textureType);
    }
    else if (id.type == DxsoRegisterType::Input
          || id.type == DxsoRegisterType::Texture
          || id.type == DxsoRegisterType::Output) {
      DxsoSemantic semantic = ctx.dcl.semantic;

      uint32_t vIndex = id.num;

      if (m_programInfo.type() == DxsoProgramTypes::PixelShader) {
        // Semantic in PS < 3 is based upon id.
        if (m_programInfo.majorVersion() < 3) {
          // Account for the two color registers.
          if (id.type == DxsoRegisterType::Texture)
            vIndex += 2;

          semantic = DxsoSemantic{
            id.type == DxsoRegisterType::Texture ? DxsoUsage::Texcoord : DxsoUsage::Color,
            id.num };
        }
      }

      this->emitDclInterface(
        id.type != DxsoRegisterType::Output,
        vIndex,
        semantic,
        ctx.dst.mask,
        ctx.dst.centroid);
    }
    else {
      //Logger::warn(str::format("DxsoCompiler::emitDcl: unhandled register type ", id.type));
    }
  }

  void DxsoCompiler::emitDef(const DxsoInstructionContext& ctx) {
    switch (ctx.instruction.opcode) {
      case DxsoOpcode::Def:  emitDefF(ctx); break;
      case DxsoOpcode::DefI: emitDefI(ctx); break;
      case DxsoOpcode::DefB: emitDefB(ctx); break;
      default:
        throw DxvkError("DxsoCompiler::emitDef: Invalid definition opcode");
        break;
    }
  }

  void DxsoCompiler::emitDefF(const DxsoInstructionContext& ctx) {
    const float* data = ctx.def.float32;

    uint32_t constId = m_module.constvec4f32(data[0], data[1], data[2], data[3]);
    m_cFloat.at(ctx.dst.id.num) = constId;

    std::string name = str::format("cF", ctx.dst.id.num, "_def");
    m_module.setDebugName(constId, name.c_str());

    DxsoDefinedConstant constant;
    constant.uboIdx = ctx.dst.id.num;
    for (uint32_t i = 0; i < 4; i++)
      constant.float32[i] = data[i];
    m_constants.push_back(constant);
    m_maxDefinedConstant = std::max(constant.uboIdx, m_maxDefinedConstant);
  }

  void DxsoCompiler::emitDefI(const DxsoInstructionContext& ctx) {
    const int32_t* data = ctx.def.int32;

    uint32_t constId = m_module.constvec4i32(data[0], data[1], data[2], data[3]);
    m_cInt.at(ctx.dst.id.num) = constId;

    std::string name = str::format("cI", ctx.dst.id.num, "_def");
    m_module.setDebugName(constId, name.c_str());
  }

  void DxsoCompiler::emitDefB(const DxsoInstructionContext& ctx) {
    const int32_t* data = ctx.def.int32;

    uint32_t constId = m_module.constBool(data[0] != 0);
    m_cBool.at(ctx.dst.id.num) = constId;

    std::string name = str::format("cB", ctx.dst.id.num, "_def");
    m_module.setDebugName(constId, name.c_str());
  }


  bool DxsoCompiler::isScalarRegister(DxsoRegisterId id) {
    return id == DxsoRegisterId{DxsoRegisterType::DepthOut, 0}
        || id == DxsoRegisterId{DxsoRegisterType::RasterizerOut, RasterOutPointSize}
        || id == DxsoRegisterId{DxsoRegisterType::RasterizerOut, RasterOutFog};
  }


  void DxsoCompiler::emitMov(const DxsoInstructionContext& ctx) {
    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    DxsoRegMask mask = ctx.dst.mask;

    if (isScalarRegister(ctx.dst.id))
      mask = DxsoRegMask(true, false, false, false);

    DxsoRegisterValue src0 = emitRegisterLoad(ctx.src[0], mask);

    DxsoRegisterValue result;
    result.type.ctype  = dst.type.ctype;
    result.type.ccount = mask.popCount();

    const uint32_t typeId = getVectorTypeId(result.type);

    if (dst.type.ctype != src0.type.ctype) {
      // We have Mova for this... but it turns out Mov has the same behaviour in d3d9!

      // Convert float -> int32_t
      // and vice versa
      if (dst.type.ctype == DxsoScalarType::Sint32) {
        // We need to floor for VS 1.1 and below, the documentation is a dirty stinking liar.
        if (m_programInfo.majorVersion() < 2 && m_programInfo.minorVersion() < 2)
          result.id = m_module.opFloor(getVectorTypeId(src0.type), src0.id);
        else
          result.id = m_module.opRound(getVectorTypeId(src0.type), src0.id);

        result.id = m_module.opConvertFtoS(typeId, result.id);
      }
      else // Float32
        result.id = m_module.opConvertStoF(typeId, src0.id);
    }
    else // No special stuff needed!
      result.id = src0.id;

    this->emitDstStore(dst, result, mask, ctx.dst.saturate, emitPredicateLoad(ctx), ctx.dst.shift, ctx.dst.id);
  }

  std::array<uint32_t, 2> DxsoCompiler::emitBem(
      const DxsoInstructionContext& ctx,
      const DxsoRegisterValue& src0,
      const DxsoRegisterValue& src1) {

    // For texbem:
    //  src0 = tc(m), src1 = t(n), dst.x = u', dst.y = v'

    // dst.x = src0.x + [bm00(m) * src1.x + bm10(m) * src1.y]
    // dst.y = src0.y + [bm01(m) * src1.x + bm11(m) * src1.y]

    // But we flipped the bm indices so we can use dot here...

    // dst.x = src0.x + dot(bm0, src1)
    // dst.y = src0.y + dot(bm1, src1)

    std::array<uint32_t, 2> values = { m_module.constf32(0.0f), m_module.constf32(0.0f) };

    for (uint32_t i = 0; i < 2; i++) {
      uint32_t fl_t   = getScalarTypeId(DxsoScalarType::Float32);
      uint32_t vec2_t = getVectorTypeId({ DxsoScalarType::Float32, 2 });
      std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };

      uint32_t tc_m_n = m_module.opCompositeExtract(fl_t, src0.id, 1, &i);

      uint32_t offset = m_module.constu32(D3D9SharedPSStages_Count * ctx.dst.id.num + D3D9SharedPSStages_BumpEnvMat0 + i);
      uint32_t bm     = m_module.opAccessChain(m_module.defPointerType(vec2_t, spv::StorageClassUniform),
                                              m_ps.sharedState, 1, &offset);
                bm    = m_module.opLoad(vec2_t, bm);

      uint32_t t      = m_module.opVectorShuffle(vec2_t, src1.id, src1.id, 2, indices.data());

      uint32_t dot    = m_module.opDot(fl_t, bm, t);

      values[i]       = m_module.opFAdd(fl_t, tc_m_n, dot);
    }
    return values;
  }


  void DxsoCompiler::emitVectorAlu(const DxsoInstructionContext& ctx) {
    const auto& src = ctx.src;

    DxsoRegMask mask = ctx.dst.mask;

    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    if (isScalarRegister(ctx.dst.id))
      mask = DxsoRegMask(true, false, false, false);

    DxsoRegisterValue result;
    result.type.ctype  = dst.type.ctype;
    result.type.ccount = mask.popCount();

    DxsoVectorType scalarType = result.type;
    scalarType.ccount = 1;

    const uint32_t typeId       = getVectorTypeId(result.type);
    const uint32_t scalarTypeId = getVectorTypeId(scalarType);

    const DxsoOpcode opcode = ctx.instruction.opcode;
    switch (opcode) {
      case DxsoOpcode::Add:
        result.id = m_module.opFAdd(typeId,
          emitRegisterLoad(src[0], mask).id,
          emitRegisterLoad(src[1], mask).id);
        break;
      case DxsoOpcode::Sub:
        result.id = m_module.opFSub(typeId,
          emitRegisterLoad(src[0], mask).id,
          emitRegisterLoad(src[1], mask).id);
        break;
      case DxsoOpcode::Mad:
        result.id = emitMad(
          emitRegisterLoad(src[0], mask),
          emitRegisterLoad(src[1], mask),
          emitRegisterLoad(src[2], mask)).id;
        break;
      case DxsoOpcode::Mul:
        result.id = emitMul(
          emitRegisterLoad(src[0], mask),
          emitRegisterLoad(src[1], mask)).id;
        break;
      case DxsoOpcode::Rcp:
        result.id = m_module.opFDiv(typeId,
          m_module.constfReplicant(1.0f, result.type.ccount),
          emitRegisterLoad(src[0], mask).id);

        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          result.id = m_module.opNMin(typeId, result.id,
            m_module.constfReplicant(std::numeric_limits<float>::max(), result.type.ccount));
        }
        break;
      case DxsoOpcode::Rsq: 
        result.id = m_module.opFAbs(typeId,
          emitRegisterLoad(src[0], mask).id);

        result.id = m_module.opInverseSqrt(typeId,
          result.id);

        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          result.id = m_module.opNMin(typeId, result.id,
            m_module.constfReplicant(std::numeric_limits<float>::max(), result.type.ccount));
        }
        break;
      case DxsoOpcode::Dp3: {
        DxsoRegMask srcMask(true, true, true, false);
        result = emitDot(
          emitRegisterLoad(src[0], srcMask),
          emitRegisterLoad(src[1], srcMask));
        break;
      }
      case DxsoOpcode::Dp4:
        result = emitDot(
          emitRegisterLoad(src[0], IdentityWriteMask),
          emitRegisterLoad(src[1], IdentityWriteMask));
        break;
      case DxsoOpcode::Slt:
      case DxsoOpcode::Sge: {
        const uint32_t boolTypeId =
          getVectorTypeId({ DxsoScalarType::Bool, result.type.ccount });

        uint32_t cmpResult = opcode == DxsoOpcode::Slt
          ? m_module.opFOrdLessThan        (boolTypeId, emitRegisterLoad(src[0], mask).id, emitRegisterLoad(src[1], mask).id)
          : m_module.opFOrdGreaterThanEqual(boolTypeId, emitRegisterLoad(src[0], mask).id, emitRegisterLoad(src[1], mask).id);

        result.id = m_module.opSelect(typeId, cmpResult,
          m_module.constfReplicant(1.0f, result.type.ccount),
          m_module.constfReplicant(0.0f, result.type.ccount));
        break;
      }
      case DxsoOpcode::Min:
        result.id = m_module.opFMin(typeId,
          emitRegisterLoad(src[0], mask).id,
          emitRegisterLoad(src[1], mask).id);
        break;
      case DxsoOpcode::Max:
        result.id = m_module.opFMax(typeId,
          emitRegisterLoad(src[0], mask).id,
          emitRegisterLoad(src[1], mask).id);
        break;
      case DxsoOpcode::ExpP:
        if (m_programInfo.majorVersion() < 2) {
          DxsoRegMask srcMask(true, false, false, false);
          uint32_t src0 = emitRegisterLoad(src[0], srcMask).id;

          uint32_t index = 0;

          std::array<uint32_t, 4> resultIndices;

          if (mask[0]) resultIndices[index++] = m_module.opExp2(scalarTypeId, m_module.opFloor(scalarTypeId, src0));
          if (mask[1]) resultIndices[index++] = m_module.opFSub(scalarTypeId, src0, m_module.opFloor(scalarTypeId, src0));
          if (mask[2]) resultIndices[index++] = m_module.opExp2(scalarTypeId, src0);
          if (mask[3]) resultIndices[index++] = m_module.constf32(1.0f);

          if (result.type.ccount == 1)
            result.id = resultIndices[0];
          else
            result.id = m_module.opCompositeConstruct(typeId, result.type.ccount, resultIndices.data());

        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          result.id = m_module.opNMin(typeId, result.id,
            m_module.constfReplicant(std::numeric_limits<float>::max(), result.type.ccount));
        }
          break;
        }
        [[fallthrough]];
      case DxsoOpcode::Exp:
        result.id = m_module.opExp2(typeId,
          emitRegisterLoad(src[0], mask).id);

        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          result.id = m_module.opNMin(typeId, result.id,
            m_module.constfReplicant(std::numeric_limits<float>::max(), result.type.ccount));
        }
        break;
      case DxsoOpcode::Pow: {
        uint32_t base = emitRegisterLoad(src[0], mask).id;
        base = m_module.opFAbs(typeId, base);

        uint32_t exponent = emitRegisterLoad(src[1], mask).id;

        result.id = m_module.opPow(typeId, base, exponent);

        if (m_moduleInfo.options.strictPow && m_moduleInfo.options.d3d9FloatEmulation != D3D9FloatEmulation::Disabled) {
          DxsoRegisterValue cmp;
          cmp.type  = { DxsoScalarType::Bool, result.type.ccount };
          cmp.id    = m_module.opFOrdEqual(getVectorTypeId(cmp.type),
            exponent, m_module.constfReplicant(0.0f, cmp.type.ccount));

          result.id = m_module.opSelect(typeId, cmp.id,
            m_module.constfReplicant(1.0f, cmp.type.ccount), result.id);
        }
        break;
      }
      case DxsoOpcode::Crs: {
        DxsoRegMask vec3Mask(true, true, true, false);
        
        DxsoRegisterValue crossValue = emitCross(
          emitRegisterLoad(src[0], vec3Mask),
          emitRegisterLoad(src[1], vec3Mask));

        std::array<uint32_t, 3> indices = { 0, 0, 0 };

        uint32_t index = 0;
        for (uint32_t i = 0; i < indices.size(); i++) {
          if (mask[i])
            indices[index++] = m_module.opCompositeExtract(m_module.defFloatType(32), crossValue.id, 1, &i);
        }

        if (result.type.ccount == 1)
          result.id = indices[0];
        else
          result.id = m_module.opCompositeConstruct(typeId, result.type.ccount, indices.data());

        break;
      }
      case DxsoOpcode::Abs:
        result.id = m_module.opFAbs(typeId,
          emitRegisterLoad(src[0], mask).id);
        break;
      case DxsoOpcode::Sgn:
        result.id = m_module.opFSign(typeId,
          emitRegisterLoad(src[0], mask).id);
        break;
      case DxsoOpcode::Nrm: {
        // Nrm is 3D...
        DxsoRegMask srcMask(true, true, true, false);
        auto vec3 = emitRegisterLoad(src[0], srcMask);
        auto dot = emitDot(vec3, vec3);

        DxsoRegisterValue rcpLength;
        rcpLength.type = scalarType;
        rcpLength.id = m_module.opInverseSqrt(scalarTypeId, dot.id);
        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          rcpLength.id = m_module.opNMin(scalarTypeId, rcpLength.id, m_module.constf32(std::numeric_limits<float>::max()));
        }

        // r * rsq(r . r)
        result.id = emitMul(emitRegisterLoad(src[0], mask), emitRegisterExtend(rcpLength, mask.popCount())).id;
        break;
      }
      case DxsoOpcode::SinCos: {
        DxsoRegMask srcMask(true, false, false, false);
        uint32_t src0 = emitRegisterLoad(src[0], srcMask).id;

        std::array<uint32_t, 4> sincosVectorIndices = { 0, 0, 0, 0 };

        uint32_t index = 0;
        uint32_t type = m_module.defFloatType(32);
        uint32_t sincos = m_module.opSinCos(src0, !m_moduleInfo.options.sincosEmulation);

        uint32_t sinIndex = 0u;
        uint32_t cosIndex = 1u;

        if (mask[0])
          sincosVectorIndices[index++] = m_module.opCompositeExtract(type, sincos, 1u, &cosIndex);

        if (mask[1])
          sincosVectorIndices[index++] = m_module.opCompositeExtract(type, sincos, 1u, &sinIndex);

        for (; index < result.type.ccount; index++) {
          if (sincosVectorIndices[index] == 0)
            sincosVectorIndices[index] = m_module.constf32(0.0f);
        }
            
        if (result.type.ccount == 1)
          result.id = sincosVectorIndices[0];
        else
          result.id = m_module.opCompositeConstruct(typeId, result.type.ccount, sincosVectorIndices.data());

        break;
      }
      case DxsoOpcode::Lit: {
        DxsoRegMask srcMask(true, true, true, true);
        uint32_t srcOp = emitRegisterLoad(src[0], srcMask).id;

        const uint32_t x = 0;
        const uint32_t y = 1;
        const uint32_t w = 3;

        uint32_t srcX = m_module.opCompositeExtract(scalarTypeId, srcOp, 1, &x);
        uint32_t srcY = m_module.opCompositeExtract(scalarTypeId, srcOp, 1, &y);
        uint32_t srcW = m_module.opCompositeExtract(scalarTypeId, srcOp, 1, &w);

        uint32_t power = m_module.opFClamp(
          scalarTypeId, srcW,
          m_module.constf32(-127.9961f), m_module.constf32(127.9961f));

        std::array<uint32_t, 4> resultIndices;

        uint32_t index = 0;

        if (mask[0]) resultIndices[index++] = m_module.constf32(1.0f);
        if (mask[1]) resultIndices[index++] = m_module.opFMax(scalarTypeId, srcX, m_module.constf32(0));
        if (mask[2]) resultIndices[index++] = m_module.opPow (scalarTypeId, m_module.opFMax(scalarTypeId, srcY, m_module.constf32(0)), power);
        if (mask[3]) resultIndices[index++] = m_module.constf32(1.0f);

        const uint32_t boolType = m_module.defBoolType();
        uint32_t zTestX = m_module.opFOrdGreaterThanEqual(boolType, srcX, m_module.constf32(0));
        uint32_t zTestY = m_module.opFOrdGreaterThanEqual(boolType, srcY, m_module.constf32(0));
        uint32_t zTest  = m_module.opLogicalAnd(boolType, zTestX, zTestY);

        if (result.type.ccount > 2)
          resultIndices[2] = m_module.opSelect(
            scalarTypeId,
            zTest,
            resultIndices[2],
            m_module.constf32(0.0f));

        if (result.type.ccount == 1)
          result.id = resultIndices[0];
        else
          result.id = m_module.opCompositeConstruct(typeId, result.type.ccount, resultIndices.data());
        break;
      }
      case DxsoOpcode::Dst: {
        //dest.x = 1;
        //dest.y = src0.y * src1.y;
        //dest.z = src0.z;
        //dest.w = src1.w;

        DxsoRegMask srcMask(true, true, true, true);

        uint32_t src0 = emitRegisterLoad(src[0], srcMask).id;
        uint32_t src1 = emitRegisterLoad(src[1], srcMask).id;

        const uint32_t y = 1;
        const uint32_t z = 2;
        const uint32_t w = 3;

        DxsoRegisterValue src0Y = { scalarType, m_module.opCompositeExtract(scalarTypeId, src0, 1, &y) };
        DxsoRegisterValue src1Y = { scalarType, m_module.opCompositeExtract(scalarTypeId, src1, 1, &y) };

        uint32_t src0Z = m_module.opCompositeExtract(scalarTypeId, src0, 1, &z);
        uint32_t src1W = m_module.opCompositeExtract(scalarTypeId, src1, 1, &w);

        std::array<uint32_t, 4> resultIndices;
        resultIndices[0] = m_module.constf32(1.0f);
        resultIndices[1] = emitMul(src0Y, src1Y).id;
        resultIndices[2] = src0Z;
        resultIndices[3] = src1W;

        if (result.type.ccount == 1)
          result.id = resultIndices[0];
        else
          result.id = m_module.opCompositeConstruct(typeId, result.type.ccount, resultIndices.data());
        break;
      }
      case DxsoOpcode::LogP:
      case DxsoOpcode::Log:
        result.id = m_module.opFAbs(typeId, emitRegisterLoad(src[0], mask).id);
        result.id = m_module.opLog2(typeId, result.id);
        if (m_moduleInfo.options.d3d9FloatEmulation == D3D9FloatEmulation::Enabled) {
          result.id = m_module.opNMax(typeId, result.id,
            m_module.constfReplicant(-std::numeric_limits<float>::max(), result.type.ccount));
        }
        break;
      case DxsoOpcode::Lrp:
        result.id = emitMix(
          emitRegisterLoad(src[2], mask),
          emitRegisterLoad(src[1], mask),
          emitRegisterLoad(src[0], mask)).id;
        break;
      case DxsoOpcode::Frc:
        result.id = m_module.opFract(typeId,
          emitRegisterLoad(src[0], mask).id);
        break;
      case DxsoOpcode::Cmp: {
        const uint32_t boolTypeId =
          getVectorTypeId({ DxsoScalarType::Bool, result.type.ccount });

        uint32_t cmp = m_module.opFOrdGreaterThanEqual(
          boolTypeId,
          emitRegisterLoad(src[0], mask).id,
          m_module.constfReplicant(0.0f, result.type.ccount));

        result.id = m_module.opSelect(
          typeId, cmp,
          emitRegisterLoad(src[1], mask).id,
          emitRegisterLoad(src[2], mask).id);
        break;
      }
      case DxsoOpcode::Bem: {
        DxsoRegisterValue src0 = emitRegisterLoad(src[0], mask);
        DxsoRegisterValue src1 = emitRegisterLoad(src[1], mask);

        auto values = emitBem(ctx, src0, src1);
        result.id   = m_module.opCompositeConstruct(typeId, values.size(), values.data());
        break;
      }
      case DxsoOpcode::Cnd: {
        const uint32_t boolTypeId =
          getVectorTypeId({ DxsoScalarType::Bool, result.type.ccount });

        uint32_t cmp = m_module.opFOrdGreaterThan(
          boolTypeId,
          emitRegisterLoad(src[0], mask).id,
          m_module.constfReplicant(0.5f, result.type.ccount));

        result.id = m_module.opSelect(
          typeId, cmp,
          emitRegisterLoad(src[1], mask).id,
          emitRegisterLoad(src[2], mask).id);
        break;
      }
      case DxsoOpcode::Dp2Add: {
        DxsoRegMask dotSrcMask(true, true, false, false);
        DxsoRegMask addSrcMask(true, false, false, false);

        DxsoRegisterValue dot = emitDot(
          emitRegisterLoad(src[0], dotSrcMask),
          emitRegisterLoad(src[1], dotSrcMask));

        dot.id = m_module.opFAdd(scalarTypeId,
          dot.id, emitRegisterLoad(src[2], addSrcMask).id);

        result.id   = dot.id;
        result.type = scalarType;
        break;
      }
      case DxsoOpcode::DsX:
        result.id = m_module.opDpdx(
          typeId, emitRegisterLoad(src[0], mask).id);
        break;
      case DxsoOpcode::DsY:
        result.id = m_module.opDpdy(
          typeId, emitRegisterLoad(src[0], mask).id);
        break;
      default:
        OTDU_LOG_WARN("DxsoCompiler::emitVectorAlu: unimplemented op %u\n", opcode);
        return;
    }

    this->emitDstStore(dst, result, mask, ctx.dst.saturate, emitPredicateLoad(ctx), ctx.dst.shift, ctx.dst.id);
  }


  void DxsoCompiler::emitPredicateOp(const DxsoInstructionContext& ctx) {
    const auto& src = ctx.src;

    DxsoRegMask mask = ctx.dst.mask;

    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    DxsoRegisterValue result;
    result.type.ctype  = dst.type.ctype;
    result.type.ccount = mask.popCount();

    result.id = emitBoolComparison(
      result.type, ctx.instruction.specificData.comparison,
      emitRegisterLoad(src[0], mask).id, emitRegisterLoad(src[1], mask).id);

    this->emitValueStore(dst, result, mask, emitPredicateLoad(ctx));
  }


  void DxsoCompiler::emitMatrixAlu(const DxsoInstructionContext& ctx) {
    const DxsoOpcode opcode = ctx.instruction.opcode;

    uint32_t dotCount;
    uint32_t componentCount;

    switch (opcode) {
      case DxsoOpcode::M3x2:
        dotCount       = 3;
        componentCount = 2;
        break;
      case DxsoOpcode::M3x3:
        dotCount       = 3;
        componentCount = 3;
        break;
      case DxsoOpcode::M3x4:
        dotCount       = 3;
        componentCount = 4;
        break;
      case DxsoOpcode::M4x3:
        dotCount       = 4;
        componentCount = 3;
        break;
      case DxsoOpcode::M4x4:
        dotCount       = 4;
        componentCount = 4;
        break;
      default:
        OTDU_LOG_WARN("DxsoCompiler::emitMatrixAlu: unimplemented op %u\n", opcode);
        return;
    }

    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    // Fix the dst mask if componentCount != maskCount
    // ie. M4x3 on .xyzw.
    uint32_t maskCnt = 0;
    uint8_t mask = 0;
    for (uint32_t i = 0; i < 4 && maskCnt < componentCount; i++) {
      if (ctx.dst.mask[i]) {
        mask |= 1 << i;
        maskCnt++;
      }
    }
    DxsoRegMask dstMask = DxsoRegMask(mask);

    DxsoRegisterValue result;
    result.type.ctype  = dst.type.ctype;
    result.type.ccount = componentCount;

    const uint32_t typeId = getVectorTypeId(result.type);

    DxsoRegMask srcMask(true, true, true, dotCount == 4);
    std::array<uint32_t, 4> indices;

    DxsoRegister src0 = ctx.src[0];
    DxsoRegister src1 = ctx.src[1];

    for (uint32_t i = 0; i < componentCount; i++) {
      indices[i] = emitDot(
        emitRegisterLoad(src0, srcMask),
        emitRegisterLoad(src1, srcMask)).id;

      src1.id.num++;
    }

    result.id = m_module.opCompositeConstruct(
      typeId, componentCount, indices.data());

    this->emitDstStore(dst, result, dstMask, ctx.dst.saturate, emitPredicateLoad(ctx), ctx.dst.shift, ctx.dst.id);
  }


void DxsoCompiler::emitControlFlowGenericLoop(
          bool     count,
          uint32_t initialVar,
          uint32_t strideVar,
          uint32_t iterationCountVar) {
    const uint32_t itType = m_module.defIntType(32, 1);

    DxsoCfgBlock block;
    block.type = DxsoCfgBlockType::Loop;
    block.b_loop.labelHeader   = m_module.allocateId();
    block.b_loop.labelBegin    = m_module.allocateId();
    block.b_loop.labelContinue = m_module.allocateId();
    block.b_loop.labelBreak    = m_module.allocateId();
    block.b_loop.iteratorPtr   = m_module.newVar(
      m_module.defPointerType(itType, spv::StorageClassPrivate), spv::StorageClassPrivate);
    block.b_loop.strideVar     = strideVar;
    block.b_loop.countBackup   = 0;

    if (count) {
      DxsoBaseRegister loop;
      loop.id = { DxsoRegisterType::Loop, 0 };

      DxsoRegisterPointer loopPtr = emitGetOperandPtr(loop, nullptr);
      uint32_t loopVal = m_module.opLoad(
        getVectorTypeId(loopPtr.type), loopPtr.id);

      block.b_loop.countBackup = loopVal;

      m_module.opStore(loopPtr.id, initialVar);
    }

    m_module.setDebugName(block.b_loop.iteratorPtr, "iter");

    m_module.opStore(block.b_loop.iteratorPtr, iterationCountVar);

    m_module.opBranch(block.b_loop.labelHeader);
    m_module.opLabel (block.b_loop.labelHeader);

    m_module.opLoopMerge(
      block.b_loop.labelBreak,
      block.b_loop.labelContinue,
      spv::LoopControlMaskNone);

    m_module.opBranch(block.b_loop.labelBegin);
    m_module.opLabel (block.b_loop.labelBegin);

    uint32_t iterator = m_module.opLoad(itType, block.b_loop.iteratorPtr);
    uint32_t complete = m_module.opIEqual(m_module.defBoolType(), iterator, m_module.consti32(0));

    const uint32_t breakBlock = m_module.allocateId();
    const uint32_t mergeBlock = m_module.allocateId();

    m_module.opSelectionMerge(mergeBlock,
      spv::SelectionControlMaskNone);

    m_module.opBranchConditional(
      complete, breakBlock, mergeBlock);

    m_module.opLabel(breakBlock);

    m_module.opBranch(block.b_loop.labelBreak);

    m_module.opLabel(mergeBlock);

    iterator = m_module.opISub(itType, iterator, m_module.consti32(1));
    m_module.opStore(block.b_loop.iteratorPtr, iterator);

    m_controlFlowBlocks.push_back(block);
  }

  void DxsoCompiler::emitControlFlowGenericLoopEnd() {
    if (m_controlFlowBlocks.size() == 0
      || m_controlFlowBlocks.back().type != DxsoCfgBlockType::Loop)
      throw DxvkError("DxsoCompiler: 'EndRep' without 'Rep' or 'Loop' found");

    // Remove the block from the stack, it's closed
    const DxsoCfgBlock block = m_controlFlowBlocks.back();
    m_controlFlowBlocks.pop_back();

    if (block.b_loop.strideVar) {
      DxsoBaseRegister loop;
      loop.id = { DxsoRegisterType::Loop, 0 };

      DxsoRegisterPointer loopPtr = emitGetOperandPtr(loop, nullptr);
      uint32_t val = m_module.opLoad(
        getVectorTypeId(loopPtr.type), loopPtr.id);

      val = m_module.opIAdd(
        getVectorTypeId(loopPtr.type),
        val, block.b_loop.strideVar);

      m_module.opStore(loopPtr.id, val);
    }

    // Declare the continue block
    m_module.opBranch(block.b_loop.labelContinue);
    m_module.opLabel(block.b_loop.labelContinue);

    // Declare the merge block
    m_module.opBranch(block.b_loop.labelHeader);
    m_module.opLabel(block.b_loop.labelBreak);

    if (block.b_loop.countBackup) {
      DxsoBaseRegister loop;
      loop.id = { DxsoRegisterType::Loop, 0 };

      DxsoRegisterPointer loopPtr = emitGetOperandPtr(loop, nullptr);

      m_module.opStore(loopPtr.id, block.b_loop.countBackup);
    }
  }

  void DxsoCompiler::emitControlFlowRep(const DxsoInstructionContext& ctx) {
    DxsoRegMask srcMask(true, false, false, false);
    this->emitControlFlowGenericLoop(
      false, 0, 0,
      emitRegisterLoad(ctx.src[0], srcMask).id);
  }

  void DxsoCompiler::emitControlFlowEndRep(const DxsoInstructionContext& ctx) {
    emitControlFlowGenericLoopEnd();
  }

  void DxsoCompiler::emitControlFlowLoop(const DxsoInstructionContext& ctx) {
    const uint32_t itType = m_module.defIntType(32, 1);

    DxsoRegMask srcMask(true, true, true, false);
    uint32_t integerRegister = emitRegisterLoad(ctx.src[1], srcMask).id;
    uint32_t x = 0;
    uint32_t y = 1;
    uint32_t z = 2;

    uint32_t iterCount    = m_module.opCompositeExtract(itType, integerRegister, 1, &x);
    uint32_t initialValue = m_module.opCompositeExtract(itType, integerRegister, 1, &y);
    uint32_t strideSize   = m_module.opCompositeExtract(itType, integerRegister, 1, &z);

    this->emitControlFlowGenericLoop(
      true,
      initialValue,
      strideSize,
      iterCount);
  }

  void DxsoCompiler::emitControlFlowEndLoop(const DxsoInstructionContext& ctx) {
    this->emitControlFlowGenericLoopEnd();
  }

  void DxsoCompiler::emitControlFlowBreak(const DxsoInstructionContext& ctx) {
    DxsoCfgBlock* cfgBlock =
      cfgFindBlock({ DxsoCfgBlockType::Loop });

    if (cfgBlock == nullptr)
      throw DxvkError("DxbcCompiler: 'Break' outside 'Rep' or 'Loop' found");

    m_module.opBranch(cfgBlock->b_loop.labelBreak);

    // Subsequent instructions assume that there is an open block
    const uint32_t labelId = m_module.allocateId();
    m_module.opLabel(labelId);
  }

  void DxsoCompiler::emitControlFlowBreakC(const DxsoInstructionContext& ctx) {
    DxsoCfgBlock* cfgBlock =
      cfgFindBlock({ DxsoCfgBlockType::Loop });

    if (cfgBlock == nullptr)
      throw DxvkError("DxbcCompiler: 'BreakC' outside 'Rep' or 'Loop' found");

    DxsoRegMask srcMask(true, false, false, false);
    auto a = emitRegisterLoad(ctx.src[0], srcMask);
    auto b = emitRegisterLoad(ctx.src[1], srcMask);

    uint32_t result = this->emitBoolComparison(
      { DxsoScalarType::Bool, a.type.ccount },
      ctx.instruction.specificData.comparison,
      a.id, b.id);

    // We basically have to wrap this into an 'if' block
    const uint32_t breakBlock = m_module.allocateId();
    const uint32_t mergeBlock = m_module.allocateId();

    m_module.opSelectionMerge(mergeBlock,
      spv::SelectionControlMaskNone);

    m_module.opBranchConditional(
      result, breakBlock, mergeBlock);

    m_module.opLabel(breakBlock);

    m_module.opBranch(cfgBlock->b_loop.labelBreak);

    m_module.opLabel(mergeBlock);
  }

  void DxsoCompiler::emitControlFlowIf(const DxsoInstructionContext& ctx) {
    const auto opcode = ctx.instruction.opcode;

    uint32_t result;

    DxsoRegMask srcMask(true, false, false, false);
    if (opcode == DxsoOpcode::Ifc) {
      auto a = emitRegisterLoad(ctx.src[0], srcMask);
      auto b = emitRegisterLoad(ctx.src[1], srcMask);

      result = this->emitBoolComparison(
        { DxsoScalarType::Bool, a.type.ccount },
        ctx.instruction.specificData.comparison,
        a.id, b.id);
    } else
      result = emitRegisterLoad(ctx.src[0], srcMask).id;

    // Declare the 'if' block. We do not know if there
    // will be an 'else' block or not, so we'll assume
    // that there is one and leave it empty otherwise.
    DxsoCfgBlock block;
    block.type = DxsoCfgBlockType::If;
    block.b_if.ztestId   = result;
    block.b_if.labelIf   = m_module.allocateId();
    block.b_if.labelElse = 0;
    block.b_if.labelEnd  = m_module.allocateId();
    block.b_if.headerPtr = m_module.getInsertionPtr();
    m_controlFlowBlocks.push_back(block);

    // We'll insert the branch instruction when closing
    // the block, since we don't know whether or not an
    // else block is needed right now.
    m_module.opLabel(block.b_if.labelIf);
  }

  void DxsoCompiler::emitControlFlowElse(const DxsoInstructionContext& ctx) {
    if (m_controlFlowBlocks.size() == 0
     || m_controlFlowBlocks.back().type != DxsoCfgBlockType::If
     || m_controlFlowBlocks.back().b_if.labelElse != 0)
      throw DxvkError("DxsoCompiler: 'Else' without 'If' found");
    
    // Set the 'Else' flag so that we do
    // not insert a dummy block on 'EndIf'
    DxsoCfgBlock& block = m_controlFlowBlocks.back();
    block.b_if.labelElse = m_module.allocateId();
    
    // Close the 'If' block by branching to
    // the merge block we declared earlier
    m_module.opBranch(block.b_if.labelEnd);
    m_module.opLabel (block.b_if.labelElse);
  }

  void DxsoCompiler::emitControlFlowEndIf(const DxsoInstructionContext& ctx) {
    if (m_controlFlowBlocks.size() == 0
     || m_controlFlowBlocks.back().type != DxsoCfgBlockType::If)
      throw DxvkError("DxsoCompiler: 'EndIf' without 'If' found");
    
    // Remove the block from the stack, it's closed
    DxsoCfgBlock block = m_controlFlowBlocks.back();
    m_controlFlowBlocks.pop_back();
    
    // Write out the 'if' header
    m_module.beginInsertion(block.b_if.headerPtr);
    
    m_module.opSelectionMerge(
      block.b_if.labelEnd,
      spv::SelectionControlMaskNone);
    
    m_module.opBranchConditional(
      block.b_if.ztestId,
      block.b_if.labelIf,
      block.b_if.labelElse != 0
        ? block.b_if.labelElse
        : block.b_if.labelEnd);
    
    m_module.endInsertion();
    
    // End the active 'if' or 'else' block
    m_module.opBranch(block.b_if.labelEnd);
    m_module.opLabel (block.b_if.labelEnd);
  }


  void DxsoCompiler::emitTexCoord(const DxsoInstructionContext& ctx) {
    DxsoRegisterValue result;

    if (m_programInfo.majorVersion() == 1 && m_programInfo.minorVersion() == 4) {
      // TexCrd Op (PS 1.4)
      result = emitRegisterLoad(ctx.src[0], ctx.dst.mask);
    } else {
      // TexCoord Op (PS 1.0 - PS 1.3)
      DxsoRegister texcoord;
      texcoord.id.type = DxsoRegisterType::PixelTexcoord;
      texcoord.id.num  = ctx.dst.id.num;

      result = emitRegisterLoadRaw(texcoord, nullptr);
      // Saturate
      result = emitSaturate(result);
      // w = 1.0f
      uint32_t wIndex = 3;
      result.id = m_module.opCompositeInsert(getVectorTypeId(result.type),
        m_module.constf32(1.0f),
        result.id,
        1, &wIndex);
    }

    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    this->emitDstStore(dst, result, ctx.dst.mask, ctx.dst.saturate, emitPredicateLoad(ctx), ctx.dst.shift, ctx.dst.id);
  }

  void DxsoCompiler::emitTextureSample(const DxsoInstructionContext& ctx) {
    DxsoRegisterPointer dst = emitGetOperandPtr(ctx.dst);

    const DxsoOpcode opcode = ctx.instruction.opcode;

    DxsoRegisterValue texcoordVar;
    uint32_t samplerIdx = 0u;

    DxsoRegMask vec3Mask(true, true, true,  false);
    DxsoRegMask srcMask (true, true, true,  true);

    auto DoProjection = [&](DxsoRegisterValue coord, bool switchProjRes) {
      uint32_t bool_t = m_module.defBoolType();
      uint32_t texcoord_t = getVectorTypeId(coord.type);

      uint32_t w = 3;

      uint32_t projScalar = m_module.opCompositeExtract(
        m_module.defFloatType(32), coord.id, 1, &w);

      projScalar = m_module.opFDiv(m_module.defFloatType(32), m_module.constf32(1.0), projScalar);
      uint32_t projResult = m_module.opVectorTimesScalar(texcoord_t, coord.id, projScalar);

      if (switchProjRes) {
        uint32_t shouldProj = m_spec.get(m_module, m_specUbo, SpecProjectionType, samplerIdx, 1);
        shouldProj = m_module.opINotEqual(bool_t, shouldProj, m_module.constu32(0));

        uint32_t bvec4_t = m_module.defVectorType(bool_t, 4);
        std::array<uint32_t, 4> indices = { shouldProj, shouldProj, shouldProj, shouldProj };
        shouldProj = m_module.opCompositeConstruct(bvec4_t, indices.size(), indices.data());

        return m_module.opSelect(texcoord_t, shouldProj, projResult, coord.id);
      } else {
        return projResult;
      }
    };

    if (opcode == DxsoOpcode::TexM3x2Tex || opcode == DxsoOpcode::TexM3x3Tex || opcode == DxsoOpcode::TexM3x3Spec || opcode == DxsoOpcode::TexM3x3VSpec) {
      const uint32_t count = opcode == DxsoOpcode::TexM3x2Tex ? 2 : 3;

      auto n = emitRegisterLoad(ctx.src[0], vec3Mask);

      std::array<uint32_t, 4> indices = { 0, 0, m_module.constf32(0.0f), m_module.constf32(0.0f) };
      for (uint32_t i = 0; i < count; i++) {
        auto reg = ctx.dst;
        reg.id.num -= (count - 1) - i;
        auto m = emitRegisterLoadTexcoord(reg, vec3Mask);

        indices[i] = emitDot(m, n).id;
      }

      if (opcode == DxsoOpcode::TexM3x3Spec || opcode == DxsoOpcode::TexM3x3VSpec) {
        uint32_t vec3Type = getVectorTypeId({ DxsoScalarType::Float32, 3 });
        uint32_t normal = m_module.opCompositeConstruct(vec3Type, 3, indices.data());

        uint32_t eyeRay;
        // VSpec -> Create eye ray from .w of last 3 tex coords (m, m-1, m-2)
        // Spec -> Get eye ray from src[1]
        if (opcode == DxsoOpcode::TexM3x3VSpec) {
          DxsoRegMask wMask(false, false, false, true);

          std::array<uint32_t, 3> eyeRayIndices;
          for (uint32_t i = 0; i < 3; i++) {
            auto reg = ctx.dst;
            reg.id.num -= (count - 1) - i;
            eyeRayIndices[i] = emitRegisterLoadTexcoord(reg, wMask).id;
          }

          eyeRay = m_module.opCompositeConstruct(vec3Type, eyeRayIndices.size(), eyeRayIndices.data());
        }
        else
          eyeRay = emitRegisterLoad(ctx.src[1], vec3Mask).id;

        eyeRay = m_module.opNormalize(vec3Type, eyeRay);
        normal = m_module.opNormalize(vec3Type, normal);
        uint32_t reflection = m_module.opReflect(vec3Type, eyeRay, normal);
        reflection = m_module.opFNegate(vec3Type, reflection);

        for (uint32_t i = 0; i < 3; i++)
          indices[i] = m_module.opCompositeExtract(m_module.defFloatType(32), reflection, 1, &i);
      }

      texcoordVar.type = { DxsoScalarType::Float32, 4 };
      texcoordVar.id   = m_module.opCompositeConstruct(getVectorTypeId(texcoordVar.type), indices.size(), indices.data());
      
      samplerIdx = ctx.dst.id.num;
    }
    else if (opcode == DxsoOpcode::TexBem || opcode == DxsoOpcode::TexBemL) {
      auto m = emitRegisterLoadTexcoord(ctx.dst, srcMask);
      auto n = emitRegisterLoad(ctx.src[0], srcMask);

      texcoordVar = m;
      samplerIdx = ctx.dst.id.num;

      // The projection (/.w) happens before this...
      // Of course it does...
      texcoordVar.id  = DoProjection(texcoordVar, true);
      auto values     = emitBem(ctx, texcoordVar, n);
      for (uint32_t i = 0; i < 2; i++)
        texcoordVar.id = m_module.opCompositeInsert(getVectorTypeId(texcoordVar.type), values[i], texcoordVar.id, 1, &i);
    }
    else if (opcode == DxsoOpcode::TexReg2Ar) {
      texcoordVar = emitRegisterLoad(ctx.src[0], srcMask);
      texcoordVar = emitRegisterSwizzle(texcoordVar, DxsoRegSwizzle(3, 0, 0, 0), srcMask);

      samplerIdx = ctx.dst.id.num;
    }
    else if (opcode == DxsoOpcode::TexReg2Gb) {
      texcoordVar = emitRegisterLoad(ctx.src[0], srcMask);
      texcoordVar = emitRegisterSwizzle(texcoordVar, DxsoRegSwizzle(1, 2, 2, 2), srcMask);

      samplerIdx = ctx.dst.id.num;
    }
    else if (opcode == DxsoOpcode::TexReg2Rgb) {
      texcoordVar = emitRegisterLoad(ctx.src[0], srcMask);
      texcoordVar = emitRegisterSwizzle(texcoordVar, DxsoRegSwizzle(0, 1, 2, 2), srcMask);

      samplerIdx = ctx.dst.id.num;
    }
    else if (opcode == DxsoOpcode::TexDp3Tex) {
      auto m = emitRegisterLoadTexcoord(ctx.dst,    vec3Mask);
      auto n = emitRegisterLoad(ctx.src[0], vec3Mask);

      auto dot = emitDot(m, n);

      std::array<uint32_t, 4> indices = { dot.id, m_module.constf32(0.0f), m_module.constf32(0.0f), m_module.constf32(0.0f) };

      texcoordVar.type = { DxsoScalarType::Float32, 4 };
      texcoordVar.id   = m_module.opCompositeConstruct(getVectorTypeId(texcoordVar.type),
        indices.size(), indices.data());

      samplerIdx  = ctx.dst.id.num;
    }
    else {
      if (m_programInfo.majorVersion() >= 2) { // SM 2.0+
        texcoordVar = emitRegisterLoad(ctx.src[0], srcMask);
        samplerIdx  = ctx.src[1].id.num;
      } else if (
        m_programInfo.majorVersion() == 1
     && m_programInfo.minorVersion() == 4) { // SM 1.4
        texcoordVar = emitRegisterLoad(ctx.src[0], srcMask);
        samplerIdx  = ctx.dst.id.num;
      }
      else { // SM 1.0-1.3
        texcoordVar = emitRegisterLoadTexcoord(ctx.dst, srcMask);
        samplerIdx  = ctx.dst.id.num;
      }
    }

    // SM < 1.x does not have dcl sampler type.
    if (m_programInfo.majorVersion() < 2 && !m_samplers[samplerIdx].color[SamplerTypeTexture2D].imageVarId)
      emitDclSampler(samplerIdx, DxsoTextureType::Texture2D);

    DxsoSampler sampler = m_samplers.at(samplerIdx);

    auto SampleImage = [this, opcode, dst, ctx, samplerIdx, DoProjection](DxsoRegisterValue texcoordVar, DxsoSamplerInfo& sampler, bool depth, DxsoSamplerType samplerType, uint32_t isNull) {
      DxsoRegisterValue result;
      result.type.ctype  = dst.type.ctype;
      result.type.ccount = depth ? 1 : 4;

      const uint32_t typeId = getVectorTypeId(result.type);

      SpirvImageOperands imageOperands;
      if (m_programInfo.type() == DxsoProgramTypes::VertexShader) {
        imageOperands.sLod = m_module.constf32(0.0f);
        imageOperands.flags |= spv::ImageOperandsLodMask;
      }

      if (opcode == DxsoOpcode::TexLdl) {
        uint32_t w = 3;
        imageOperands.sLod = m_module.opCompositeExtract(
          m_module.defFloatType(32), texcoordVar.id, 1, &w);
        imageOperands.flags |= spv::ImageOperandsLodMask;
      }

      if (opcode == DxsoOpcode::TexLdd) {
        DxsoRegMask gradMask(true, true, sampler.dimensions == 3, false);
        imageOperands.flags |= spv::ImageOperandsGradMask;
        imageOperands.sGradX = emitRegisterLoad(ctx.src[2], gradMask).id;
        imageOperands.sGradY = emitRegisterLoad(ctx.src[3], gradMask).id;
      }

      if (opcode == DxsoOpcode::Tex
        && m_programInfo.majorVersion() >= 2) {
        if (ctx.instruction.specificData.texld == DxsoTexLdMode::Project) {
          texcoordVar.id = DoProjection(texcoordVar, false);
        }
        else if (ctx.instruction.specificData.texld == DxsoTexLdMode::Bias) {
          uint32_t w = 3;
          imageOperands.sLodBias = m_module.opCompositeExtract(
            m_module.defFloatType(32), texcoordVar.id, 1, &w);
          imageOperands.flags |= spv::ImageOperandsBiasMask;
        }
      }

      // We already handled this for TexBem(L)
      if (m_programInfo.majorVersion() < 2 && samplerType != SamplerTypeTextureCube && opcode != DxsoOpcode::TexBem && opcode != DxsoOpcode::TexBemL) {
        texcoordVar.id = DoProjection(texcoordVar, true);
      }

      uint32_t bool_t = m_module.defBoolType();

      uint32_t reference = 0;
      if (depth) {
        uint32_t fType = m_module.defFloatType(32);
        uint32_t component = sampler.dimensions;
        reference = m_module.opCompositeExtract(
          fType, texcoordVar.id, 1, &component);

        // [D3D8] Scale Dref from [0..(2^N - 1)] for D24S8 and D16 if Dref scaling is enabled
        if (m_moduleInfo.options.drefScaling) {
          uint32_t drefScale       = m_module.constf32(GetDrefScaleFactor(m_moduleInfo.options.drefScaling));
          reference                = m_module.opFMul(fType, reference, drefScale);
        }

        // Clamp Dref to [0..1] for D32F emulating UNORM textures 
        uint32_t clampDref = m_spec.get(m_module, m_specUbo, SpecDrefClamp, samplerIdx, 1);
        clampDref = m_module.opINotEqual(bool_t, clampDref, m_module.constu32(0));
        uint32_t clampedDref = m_module.opFClamp(fType, reference, m_module.constf32(0.0f), m_module.constf32(1.0f));
        reference = m_module.opSelect(fType, clampDref, clampedDref, reference);
      }

      uint32_t fetch4 = 0;
      if (m_programInfo.type() == DxsoProgramType::PixelShader && samplerType != SamplerTypeTexture3D) {
        fetch4 = m_spec.get(m_module, m_specUbo, SpecFetch4, samplerIdx, 1);

        fetch4 = m_module.opINotEqual(bool_t, fetch4, m_module.constu32(0));

        uint32_t bvec4_t = m_module.defVectorType(bool_t, 4);
        std::array<uint32_t, 4> indices = { fetch4, fetch4, fetch4, fetch4 };
        fetch4 = m_module.opCompositeConstruct(bvec4_t, indices.size(), indices.data());
      }

      result.id = this->emitSample(
        typeId,
        sampler,
        texcoordVar,
        reference,
        fetch4,
        imageOperands);

      // If we are sampling depth we've already specc'ed this!
      // This path is always size 4 because it only hits on color.
      if (isNull != 0) {
        uint32_t bool_t = m_module.defBoolType();
        uint32_t bvec4_t = m_module.defVectorType(bool_t, 4);
        std::array<uint32_t, 4> indices = { isNull, isNull, isNull, isNull };
        isNull = m_module.opCompositeConstruct(bvec4_t, indices.size(), indices.data());
        result.id = m_module.opSelect(typeId, isNull, m_module.constvec4f32(0.0f, 0.0f, 0.0f, 1.0f), result.id);
      }

      // Apply operand swizzle to the operand value
      if (m_programInfo.majorVersion() >= 2) // SM 2.0+
        result = emitRegisterSwizzle(result, ctx.src[1].swizzle, ctx.dst.mask);
      else
        result = emitRegisterSwizzle(result, IdentitySwizzle, ctx.dst.mask);

      if (opcode == DxsoOpcode::TexBemL) {
        uint32_t float_t = m_module.defFloatType(32);

        uint32_t index = m_module.constu32(D3D9SharedPSStages_Count * ctx.dst.id.num + D3D9SharedPSStages_BumpEnvLScale);
        uint32_t lScale = m_module.opAccessChain(m_module.defPointerType(float_t, spv::StorageClassUniform),
                                                 m_ps.sharedState, 1, &index);
                 lScale = m_module.opLoad(float_t, lScale);

                 index = m_module.constu32(D3D9SharedPSStages_Count * ctx.dst.id.num + D3D9SharedPSStages_BumpEnvLOffset);
        uint32_t lOffset = m_module.opAccessChain(m_module.defPointerType(float_t, spv::StorageClassUniform),
                                                  m_ps.sharedState, 1, &index);
                 lOffset = m_module.opLoad(float_t, lOffset);

        uint32_t zIndex = 2;
        uint32_t scale = m_module.opCompositeExtract(float_t, result.id, 1, &zIndex);
                 scale = m_module.opFMul(float_t, scale, lScale);
                 scale = m_module.opFAdd(float_t, scale, lOffset);
                 scale = m_module.opFClamp(float_t, scale, m_module.constf32(0.0f), m_module.constf32(1.0));

        result.id = m_module.opVectorTimesScalar(getVectorTypeId(result.type), result.id, scale);
      }

      this->emitDstStore(dst, result, ctx.dst.mask, ctx.dst.saturate, emitPredicateLoad(ctx), ctx.dst.shift, ctx.dst.id);
    };

    auto SampleType = [&](DxsoSamplerType samplerType) {
      uint32_t bitOffset = m_programInfo.type() == DxsoProgramTypes::VertexShader
        ? samplerIdx + FirstVSSamplerSlot
        : samplerIdx;

      uint32_t isNull = m_spec.get(m_module, m_specUbo, SpecSamplerNull, bitOffset, 1);
      isNull = m_module.opINotEqual(m_module.defBoolType(), isNull, m_module.constu32(0));

      // Only do the check for depth comp. samplers
      // if we aren't a 3D texture
      if (samplerType != SamplerTypeTexture3D) {
        uint32_t colorLabel  = m_module.allocateId();
        uint32_t depthLabel  = m_module.allocateId();
        uint32_t endLabel    = m_module.allocateId();

        uint32_t isDepth = m_spec.get(m_module, m_specUbo, SpecSamplerDepthMode, bitOffset, 1);
        isDepth = m_module.opINotEqual(m_module.defBoolType(), isDepth, m_module.constu32(0));

        m_module.opSelectionMerge(endLabel, spv::SelectionControlMaskNone);
        m_module.opBranchConditional(isDepth, depthLabel, colorLabel);

        m_module.opLabel(colorLabel);
        SampleImage(texcoordVar, sampler.color[samplerType], false, samplerType, isNull);
        m_module.opBranch(endLabel);

        m_module.opLabel(depthLabel);
        // No spec constant as if we are unbound we always fall down the color path.
        SampleImage(texcoordVar, sampler.depth[samplerType], true, samplerType, 0);
        m_module.opBranch(endLabel);

        m_module.opLabel(endLabel);
      }
      else
        SampleImage(texcoordVar, sampler.color[samplerType], false, samplerType, isNull);
    };

    if (m_programInfo.majorVersion() >= 2 && !m_moduleInfo.options.forceSamplerTypeSpecConstants) {
      DxsoSamplerType samplerType =
        SamplerTypeFromTextureType(sampler.type);

      SampleType(samplerType);
    }
    else {
      std::array<SpirvSwitchCaseLabel, 3> typeCaseLabels = {{
        { uint32_t(SamplerTypeTexture2D),           m_module.allocateId() },
        { uint32_t(SamplerTypeTexture3D),           m_module.allocateId() },
        { uint32_t(SamplerTypeTextureCube),         m_module.allocateId() },
      }};

      uint32_t switchEndLabel = m_module.allocateId();
      uint32_t type = m_spec.get(m_module, m_specUbo, SpecSamplerType, samplerIdx * 2, 2);

      m_module.opSelectionMerge(switchEndLabel, spv::SelectionControlMaskNone);
      m_module.opSwitch(type,
        typeCaseLabels[uint32_t(SamplerTypeTexture2D)].labelId,
        typeCaseLabels.size(),
        typeCaseLabels.data());

      for (const auto& label : typeCaseLabels) {
        m_module.opLabel(label.labelId);

        SampleType(DxsoSamplerType(label.literal));

        m_module.opBranch(switchEndLabel);
      }

      m_module.opLabel(switchEndLabel);
    }
  }

  void DxsoCompiler::emitTextureKill(const DxsoInstructionContext& ctx) {
    DxsoRegisterValue texReg;

    if (m_programInfo.majorVersion() >= 2 ||
       (m_programInfo.majorVersion() == 1
     && m_programInfo.minorVersion() == 4)) // SM 2.0+ or 1.4
      texReg = emitRegisterLoadRaw(ctx.dst, ctx.dst.hasRelative ? &ctx.dst.relative : nullptr);
    else { // SM 1.0-1.3
      DxsoRegister texcoord;
      texcoord.id = { DxsoRegisterType::PixelTexcoord, ctx.dst.id.num };

      texReg = emitRegisterLoadRaw(texcoord, nullptr);
    }

    std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };

    // On SM1 it only works on the first 
    if (m_programInfo.majorVersion() < 2) {
      texReg.type.ccount = 3;

      texReg.id = m_module.opVectorShuffle(
        getVectorTypeId(texReg.type),
        texReg.id, texReg.id,
        texReg.type.ccount, indices.data());
    }
    else {
      // The writemask actually applies and works here...
      // (FXC doesn't generate this but it fixes broken ENB shaders)
      texReg = emitRegisterSwizzle(texReg, IdentitySwizzle, ctx.dst.mask);
    }

    const uint32_t boolVecTypeId =
      getVectorTypeId({ DxsoScalarType::Bool, texReg.type.ccount });

    uint32_t result = m_module.opFOrdLessThan(
      boolVecTypeId, texReg.id,
      m_module.constfReplicant(0.0f, texReg.type.ccount));

    if (texReg.type.ccount != 1)
      result = m_module.opAny(m_module.defBoolType(), result);

    uint32_t labelIf = m_module.allocateId();
    uint32_t labelEnd = m_module.allocateId();

    m_module.opSelectionMerge(labelEnd, spv::SelectionControlMaskNone);
    m_module.opBranchConditional(result, labelIf, labelEnd);

    m_module.opLabel(labelIf);
    m_module.opDemoteToHelperInvocation();
    m_module.opBranch(labelEnd);

    m_module.opLabel(labelEnd);
  }

  void DxsoCompiler::emitTextureDepth(const DxsoInstructionContext& ctx) {
    const uint32_t fType = m_module.defFloatType(32);

    DxsoRegMask srcMask(true, true, false, false);
    uint32_t r5 = emitRegisterLoad(ctx.src[0], srcMask).id;
    uint32_t x = 0;
    uint32_t y = 1;

    uint32_t xValue = m_module.opCompositeExtract(fType, r5, 1, &x);
    uint32_t yValue = m_module.opCompositeExtract(fType, r5, 1, &y);

    // The docs say if yValue is 0 the result is 1.0 but native drivers return
    // 0 for xValue <= 0. So we don't have to do anything special since -INF and
    // NAN get clamped to 0 at the end of the shader.
    uint32_t result = m_module.opFDiv(fType, xValue, yValue);

    DxsoBaseRegister depth;
    depth.id = { DxsoRegisterType::DepthOut, 0 };

    DxsoRegisterPointer depthPtr = emitGetOperandPtr(depth, nullptr);

    m_module.opStore(depthPtr.id, result);
  }


  uint32_t DxsoCompiler::emitSample(
          uint32_t                resultType,
          DxsoSamplerInfo&        samplerInfo,
          DxsoRegisterValue       coordinates,
          uint32_t                reference,
          uint32_t                fetch4,
    const SpirvImageOperands&     operands) {
    const bool depthCompare = reference != 0;
    const bool explicitLod  =
       (operands.flags & spv::ImageOperandsLodMask)
    || (operands.flags & spv::ImageOperandsGradMask);

    uint32_t image = m_module.opLoad(samplerInfo.imageTypeId, samplerInfo.imageVarId);
    uint32_t sampler = _LoadSampler(m_module, m_samplerArray, m_rsBlock, m_rsFirstSampler, samplerInfo.samplerIndex);

    uint32_t sampledImage = m_module.opSampledImage(samplerInfo.sampledTypeId, image, sampler);

    uint32_t val;


    if (depthCompare) {
      if (explicitLod)
        val = m_module.opImageSampleDrefExplicitLod(resultType, sampledImage, coordinates.id, reference, operands);
      else
        val = m_module.opImageSampleDrefImplicitLod(resultType, sampledImage, coordinates.id, reference, operands);
    }
    else {
      if (explicitLod)
        val = m_module.opImageSampleExplicitLod(resultType, sampledImage, coordinates.id, operands);
      else
        val = m_module.opImageSampleImplicitLod(resultType, sampledImage, coordinates.id, operands);
    }



    if (fetch4 && !depthCompare) {
      SpirvImageOperands fetch4Operands = operands;
      fetch4Operands.flags &= ~spv::ImageOperandsLodMask;
      fetch4Operands.flags &= ~spv::ImageOperandsGradMask;
      fetch4Operands.flags &= ~spv::ImageOperandsBiasMask;

      // Doesn't really work for cubes...
      // D3D9 does support gather on 3D but we cannot :<
      // Nothing probably relies on that though.
      // If we come back to this ever, make sure to handle cube/3d differences.
      if (samplerInfo.dimensions == 2) {
        uint32_t image = m_module.opImage(samplerInfo.imageTypeId, sampledImage);

        // Account for half texel offset...
        // textureSize = 1.0f / float(2 * textureSize(sampler, 0))
        DxsoRegisterValue textureSize;
        textureSize.type = { DxsoScalarType::Sint32, samplerInfo.dimensions };
        textureSize.id = m_module.opImageQuerySizeLod(getVectorTypeId(textureSize.type), image, m_module.consti32(0));
        textureSize.id = m_module.opIMul(getVectorTypeId(textureSize.type), textureSize.id, m_module.constiReplicant(2, samplerInfo.dimensions));

        textureSize.type = { DxsoScalarType::Float32, samplerInfo.dimensions };
        textureSize.id = m_module.opConvertStoF(getVectorTypeId(textureSize.type), textureSize.id);
        // HACK: Bias fetch4 half-texel offset to avoid a "grid" effect.
        // Technically we should only do that for non-powers of two
        // as only then does the imprecision need to be biased
        // towards infinity -- but that's not really worth doing...
        float numerator = 1.0f - 1.0f / 256.0f;
        textureSize.id = m_module.opFDiv(getVectorTypeId(textureSize.type), m_module.constfReplicant(numerator, samplerInfo.dimensions), textureSize.id);

        // coord => same dimensions as texture size (no cube here !)
        const std::array<uint32_t, 4> naturalIndices = { 0, 1, 2, 3 };
        coordinates.type.ccount = samplerInfo.dimensions;
        coordinates.id = m_module.opVectorShuffle(getVectorTypeId(coordinates.type), coordinates.id, coordinates.id, coordinates.type.ccount, naturalIndices.data());
        // coord += textureSize;
        coordinates.id = m_module.opFAdd(getVectorTypeId(coordinates.type), coordinates.id, textureSize.id);
      }

      uint32_t fetch4Val = m_module.opImageGather(resultType, sampledImage, coordinates.id, m_module.consti32(0), fetch4Operands);
      // B R G A swizzle... Funny D3D9 order.
      const std::array<uint32_t, 4> indices = { 2, 0, 1, 3 };
      fetch4Val = m_module.opVectorShuffle(resultType, fetch4Val, fetch4Val, indices.size(), indices.data());

      val = m_module.opSelect(resultType, fetch4, fetch4Val, val);
    }

    return val;
  }


  void DxsoCompiler::emitInputSetup() {
    uint32_t pointCoord = 0;
    D3D9PointSizeInfoPS pointInfo;

    if (m_programInfo.type() == DxsoProgramType::PixelShader) {
      pointCoord = _GetPointCoord(m_module);
      pointInfo  = _GetPointSizeInfoPS(m_spec, m_module, m_rsBlock, m_specUbo);
    }

    for (uint32_t i = 0; i < m_isgn.elemCount; i++) {
      const auto& elem = m_isgn.elems[i];
      const uint32_t slot = elem.slot;
      
      DxsoRegisterInfo info;
      info.type.ctype   = DxsoScalarType::Float32;
      info.type.ccount  = 4;
      info.type.alength = 1;
      info.sclass       = spv::StorageClassInput;

      DxsoRegisterPointer inputPtr;
      inputPtr.id          = emitNewVariable(info);
      inputPtr.type.ctype  = DxsoScalarType::Float32;
      inputPtr.type.ccount = info.type.ccount;

      m_module.decorateLocation(inputPtr.id, slot);

      if (m_programInfo.type() == DxsoProgramType::PixelShader
       && m_moduleInfo.options.forceSampleRateShading) {
        m_module.enableCapability(spv::CapabilitySampleRateShading);
        m_module.decorate(inputPtr.id, spv::DecorationSample);
      }

      std::string name =
        str::format("in_", elem.semantic.usage, elem.semantic.usageIndex);
      m_module.setDebugName(inputPtr.id, name.c_str());

      if (elem.centroid)
        m_module.decorate(inputPtr.id, spv::DecorationCentroid);

      uint32_t typeId    = this->getVectorTypeId({ DxsoScalarType::Float32, 4 });
      uint32_t ptrTypeId = m_module.defPointerType(typeId, spv::StorageClassPrivate);

      uint32_t regNumVar = m_module.constu32(elem.regNumber);

      DxsoRegisterPointer indexPtr;
      indexPtr.id   = m_module.opAccessChain(ptrTypeId, m_vArray, 1, &regNumVar);
      indexPtr.type = inputPtr.type;
      indexPtr.type.ccount = 4;

      DxsoRegisterValue indexVal = this->emitValueLoad(inputPtr);

      DxsoRegisterValue workingReg;
      workingReg.type = indexVal.type;

      workingReg.id = m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f);

      DxsoRegMask mask = elem.mask;
      if (mask.popCount() == 0)
        mask = DxsoRegMask(true, true, true, true);

      // Pixel shaders seem to load every element (or every element exported by VS)
      // regardless of the mask in the DCL used.
      //
      // I imagine this is because at one point D3D9 ASM was very literal to GPUs,
      // and the input's mask from the PS didn't correspond to anything and the
      // physical output register was filled with the extra data from the VS not
      // included in the mask on the PS, so it just worked(tm).
      //
      // Fixes a bug in Test Drive Unlimited's shadow code where it outputs o8.xyz, but the PS
      // has a decl for v8.xy and it tries to do 2D Shadow sampling (needs 3 elements .xyz) in the PS.
      // This likely occured because the compiler was not aware of shadow sampling when generating
      // the writemask for the PS.
      if (m_programInfo.type() == DxsoProgramType::PixelShader)
        mask = DxsoRegMask(true, true, true, true);

      std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };
      uint32_t count = 0;
      for (uint32_t i = 0; i < 4; i++) {
        if (mask[i]) {
          indices[i] = i + 4;
          count++;
        }
      }

      workingReg.id = m_module.opVectorShuffle(getVectorTypeId(workingReg.type),
        workingReg.id, indexVal.id, 4, indices.data());

      // We need to replace TEXCOORD inputs with gl_PointCoord
      // if D3DRS_POINTSPRITEENABLE is set.
      if (m_programInfo.type() == DxsoProgramType::PixelShader && elem.semantic.usage == DxsoUsage::Texcoord)
        workingReg.id = m_module.opSelect(getVectorTypeId(workingReg.type), pointInfo.isSprite, pointCoord, workingReg.id);

      if (m_programInfo.type() == DxsoProgramType::PixelShader && elem.semantic.usage == DxsoUsage::Color) {
        if (elem.semantic.usageIndex < 2)
          m_ps.flatShadingMask |= 1u << slot;
      }

      m_module.opStore(indexPtr.id, workingReg.id);
    }
  }


  void DxsoCompiler::emitLinkerOutputSetup() {
    bool outputtedColor0 = false;
    bool outputtedColor1 = false;

    for (uint32_t i = 0; i < m_osgn.elemCount; i++) {
      const auto& elem = m_osgn.elems[i];
      const uint32_t slot = elem.slot;

      if (elem.semantic.usage == DxsoUsage::Color) {
        if (elem.semantic.usageIndex == 0)
          outputtedColor0 = true;
        else
          outputtedColor1 = true;
      }
      
      DxsoRegisterInfo info;
      info.type.ctype   = DxsoScalarType::Float32;
      info.type.ccount  = 4;
      info.type.alength = 1;
      info.sclass       = spv::StorageClassOutput;

      spv::BuiltIn builtIn =
        semanticToBuiltIn(false, elem.semantic);

      DxsoRegisterPointer outputPtr;
      outputPtr.type.ctype  = DxsoScalarType::Float32;
      outputPtr.type.ccount = 4;

      DxsoRegMask mask = elem.mask;

      bool scalar = false;

      if (builtIn == spv::BuiltInMax) {
        outputPtr.id = emitNewVariableDefault(info,
          m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f));
        m_module.decorateLocation(outputPtr.id, slot);

        std::string name =
          str::format("out_", elem.semantic.usage, elem.semantic.usageIndex);
        m_module.setDebugName(outputPtr.id, name.c_str());
      }
      else {
        const char* name = "unknown_builtin";
        if (builtIn == spv::BuiltInPosition)
          name = "oPos";
        else if (builtIn == spv::BuiltInPointSize) {
          outputPtr.type.ccount = 1;
          info.type.ccount = 1;
          name = "oPSize";
          bool maskValues[4];
          for (uint32_t i = 0; i < 4; i++)
            maskValues[i] = i == elem.mask.firstSet();
          mask = DxsoRegMask(maskValues[0], maskValues[1], maskValues[2], maskValues[3]);
        }

        outputPtr.id = emitNewVariableDefault(info,
          m_module.constfReplicant(0.0f, info.type.ccount));

        m_module.setDebugName(outputPtr.id, name);
        m_module.decorateBuiltIn(outputPtr.id, builtIn);

        if (builtIn == spv::BuiltInPosition)
          m_vs.oPos = outputPtr;
        else if (builtIn == spv::BuiltInPointSize) {
          scalar = true;
          m_vs.oPSize = outputPtr;
        }
      }

      uint32_t typeId    = this->getVectorTypeId({ DxsoScalarType::Float32, 4 });
      uint32_t ptrTypeId = m_module.defPointerType(typeId, spv::StorageClassPrivate);

      uint32_t regNumVar = m_module.constu32(elem.regNumber);

      DxsoRegisterPointer indexPtr;
      indexPtr.id   = m_module.opAccessChain(ptrTypeId, m_oArray, 1, &regNumVar);
      indexPtr.type = outputPtr.type;
      indexPtr.type.ccount = 4;

      DxsoRegisterValue indexVal = this->emitValueLoad(indexPtr);

      DxsoRegisterValue workingReg;
      workingReg.type.ctype  = indexVal.type.ctype;
      workingReg.type.ccount = scalar ? 1 : 4;

      workingReg.id = scalar
        ? m_module.constf32(0.0f)
        : m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f);

      std::array<uint32_t, 4> indices = { 0, 1, 2, 3 };

      if (scalar) {
        workingReg.id = m_module.opCompositeExtract(getVectorTypeId(workingReg.type),
          indexVal.id, 1, indices.data());
      } else {
        if (mask.popCount() == 0)
          mask = DxsoRegMask(true, true, true, true);

        uint32_t count = 0;
        for (uint32_t i = 0; i < 4; i++) {
          if (mask[i])
            indices[count++] = i + 4;
        }


        workingReg.id = m_module.opVectorShuffle(getVectorTypeId(workingReg.type),
          workingReg.id, indexVal.id, 4, indices.data());
      }

      // Ie. 0 or 1 for diffuse and specular color
      // and for Shader Model 1 or 2
      // (because those have dedicated color registers
      // where this rule applies)
      if (elem.semantic.usage == DxsoUsage::Color &&
          elem.semantic.usageIndex < 2 &&
          m_programInfo.majorVersion() < 3)
        workingReg = emitSaturate(workingReg);

      m_module.opStore(outputPtr.id, workingReg.id);
    }

    auto OutputDefault = [&](DxsoSemantic semantic) {
      DxsoRegisterInfo info;
      info.type.ctype   = DxsoScalarType::Float32;
      info.type.ccount  = 4;
      info.type.alength = 1;
      info.sclass       = spv::StorageClassOutput;

      uint32_t slot = RegisterLinkerSlot(semantic);

      uint32_t value = semantic == DxsoSemantic{ DxsoUsage::Color, 0 }
        ? m_module.constvec4f32(1.0f, 1.0f, 1.0f, 1.0f)
        : m_module.constvec4f32(0.0f, 0.0f, 0.0f, 0.0f);


      uint32_t outputPtr = emitNewVariableDefault(info, value);

      m_module.decorateLocation(outputPtr, slot);

      std::string name =
        str::format("out_", semantic.usage, semantic.usageIndex, "_default");

      m_module.setDebugName(outputPtr, name.c_str());

      m_outputMask |= 1u << slot;
    };

    if (!outputtedColor0)
      OutputDefault(DxsoSemantic{ DxsoUsage::Color, 0 });

    if (!outputtedColor1)
      OutputDefault(DxsoSemantic{ DxsoUsage::Color, 1 });

    auto pointInfo = _GetPointSizeInfoVS(m_spec, m_module, m_vs.oPos.id, 0, 0, m_rsBlock, m_specUbo, false);

    if (m_vs.oPSize.id == 0) {
      m_vs.oPSize = this->emitRegisterPtr(
        "oPSize", DxsoScalarType::Float32, 1, 0,
        spv::StorageClassOutput, spv::BuiltInPointSize);

      uint32_t pointSize = m_module.opFClamp(m_module.defFloatType(32), pointInfo.defaultValue, pointInfo.min, pointInfo.max);

      m_module.opStore(m_vs.oPSize.id, pointSize);
    }
    else {
      uint32_t float_t = m_module.defFloatType(32);
      uint32_t pointSize = m_module.opFClamp(m_module.defFloatType(32), m_module.opLoad(float_t, m_vs.oPSize.id), pointInfo.min, pointInfo.max);
      m_module.opStore(m_vs.oPSize.id, pointSize);
    }
  }


  void DxsoCompiler::emitVsClipping() {
    uint32_t clipPlaneCountId = m_module.constu32(caps::MaxClipPlanes);
    
    uint32_t floatType = m_module.defFloatType(32);
    uint32_t vec4Type  = m_module.defVectorType(floatType, 4);
    uint32_t boolType  = m_module.defBoolType();
    
    // Declare uniform buffer containing clip planes
    uint32_t clipPlaneArray  = m_module.defArrayTypeUnique(vec4Type, clipPlaneCountId);
    uint32_t clipPlaneStruct = m_module.defStructTypeUnique(1, &clipPlaneArray);
    uint32_t clipPlaneBlock  = m_module.newVar(
      m_module.defPointerType(clipPlaneStruct, spv::StorageClassUniform),
      spv::StorageClassUniform);
    
    m_module.decorateArrayStride  (clipPlaneArray, 16);
    
    m_module.setDebugName         (clipPlaneStruct, "clip_info_t");
    m_module.setDebugMemberName   (clipPlaneStruct, 0, "clip_planes");
    m_module.decorate             (clipPlaneStruct, spv::DecorationBlock);
    m_module.memberDecorateOffset (clipPlaneStruct, 0, 0);
    
    uint32_t bindingId = computeResourceSlotId(
      m_programInfo.type(), DxsoBindingType::ConstantBuffer,
      DxsoConstantBuffers::VSClipPlanes);
    
    m_module.setDebugName         (clipPlaneBlock, "clip_info");
    m_module.decorateDescriptorSet(clipPlaneBlock, 0);
    m_module.decorateBinding      (clipPlaneBlock, bindingId);
    
    auto& binding = m_bindings.emplace_back();
    binding.set             = 0u;
    binding.binding         = bindingId;
    binding.resourceIndex   = bindingId;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.access          = VK_ACCESS_UNIFORM_READ_BIT;
    binding.flags.set(DxvkDescriptorFlag::UniformBuffer);

    // Declare output array for clip distances
    uint32_t clipDistArray = m_module.newVar(
      m_module.defPointerType(
        m_module.defArrayType(floatType, clipPlaneCountId),
        spv::StorageClassOutput),
      spv::StorageClassOutput);

    m_module.decorateBuiltIn(clipDistArray, spv::BuiltInClipDistance);

    if (m_moduleInfo.options.invariantPosition)
      m_module.decorate(m_vs.oPos.id, spv::DecorationInvariant);
    
    const uint32_t positionPtr = m_vs.oPos.id;

    // We generated a bad shader, let's not make it even worse.
    if (positionPtr == 0) {
      OTDU_LOG_WARN("Shader without Position output. Something is likely wrong here.");
      return;
    }

    // Compute clip distances
    DxsoRegisterValue position;
    position.type = { DxsoScalarType::Float32, 4 };
    position.id = m_module.opLoad(vec4Type, positionPtr);

    // Always consider clip planes enabled when doing GPL by forcing 6 for the quick value.
    uint32_t clipPlaneCount = m_spec.get(m_module, m_specUbo, SpecClipPlaneCount, 0, 32, m_module.constu32(caps::MaxClipPlanes));
    
    for (uint32_t i = 0; i < caps::MaxClipPlanes; i++) {
      std::array<uint32_t, 2> blockMembers = {{
        m_module.constu32(0),
        m_module.constu32(i),
      }};

      DxsoRegisterValue plane;
      plane.type = { DxsoScalarType::Float32, 4 };
      plane.id = m_module.opLoad(vec4Type, m_module.opAccessChain(
        m_module.defPointerType(vec4Type, spv::StorageClassUniform),
        clipPlaneBlock, blockMembers.size(), blockMembers.data()));

      DxsoRegisterValue dist = emitDot(position, plane);

      uint32_t clipPlaneEnabled = m_module.opULessThan(boolType, m_module.constu32(i), clipPlaneCount);

      uint32_t value = m_module.opSelect(floatType, clipPlaneEnabled, dist.id, m_module.constf32(0.0f));

      m_module.opStore(m_module.opAccessChain(
        m_module.defPointerType(floatType, spv::StorageClassOutput),
        clipDistArray, 1, &blockMembers[1]), value);
    }
  }


  void DxsoCompiler::setupRenderStateInfo(uint32_t samplerCount) {
    auto blockInfo = _SetupRenderStateBlock(m_module, (1u << samplerCount) - 1u);

    m_rsBlock = blockInfo.first;
    m_rsFirstSampler = blockInfo.second;

    uint32_t samplerDwordCount = (samplerCount + 1u) / 2u;

    m_samplerPushData = DxvkPushDataBlock(m_programInfo.shaderStage(), GetPushSamplerOffset(0u),
      samplerDwordCount * sizeof(uint32_t), sizeof(uint32_t), (1u << samplerDwordCount) - 1u);
  }


  uint32_t _DoFixedFunctionFog( D3D9ShaderSpecConstantManager& spec, SpirvModule& spvModule, const D3D9FogContext& fogCtx )
  {
      uint32_t floatType = spvModule.defFloatType( 32 );
      uint32_t vec3Type = spvModule.defVectorType( floatType, 3 );
      uint32_t vec4Type = spvModule.defVectorType( floatType, 4 );
      uint32_t floatPtr = spvModule.defPointerType( floatType, spv::StorageClassPushConstant );
      uint32_t vec3Ptr = spvModule.defPointerType( vec3Type, spv::StorageClassPushConstant );

      uint32_t fogColorMember = spvModule.constu32( uint32_t( D3D9RenderStateItem::FogColor ) );
      uint32_t fogColor = spvModule.opLoad( vec3Type,
                                            spvModule.opAccessChain( vec3Ptr, fogCtx.RenderState, 1, &fogColorMember ) );

      uint32_t fogScaleMember = spvModule.constu32( uint32_t( D3D9RenderStateItem::FogScale ) );
      uint32_t fogScale = spvModule.opLoad( floatType,
                                            spvModule.opAccessChain( floatPtr, fogCtx.RenderState, 1, &fogScaleMember ) );

      uint32_t fogEndMember = spvModule.constu32( uint32_t( D3D9RenderStateItem::FogEnd ) );
      uint32_t fogEnd = spvModule.opLoad( floatType,
                                          spvModule.opAccessChain( floatPtr, fogCtx.RenderState, 1, &fogEndMember ) );

      uint32_t fogDensityMember = spvModule.constu32( uint32_t( D3D9RenderStateItem::FogDensity ) );
      uint32_t fogDensity = spvModule.opLoad( floatType,
                                              spvModule.opAccessChain( floatPtr, fogCtx.RenderState, 1, &fogDensityMember ) );

      uint32_t fogMode = spec.get(
          spvModule, fogCtx.SpecUBO,
          fogCtx.IsPixel ? SpecPixelFogMode : SpecVertexFogMode );

      uint32_t fogEnabled = spec.get( spvModule, fogCtx.SpecUBO, SpecFogEnabled );
      fogEnabled = spvModule.opINotEqual( spvModule.defBoolType(), fogEnabled, spvModule.constu32( 0 ) );

      uint32_t doFog = spvModule.allocateId();
      uint32_t skipFog = spvModule.allocateId();

      uint32_t returnType = fogCtx.IsPixel ? vec4Type : floatType;
      uint32_t returnTypePtr = spvModule.defPointerType( returnType, spv::StorageClassPrivate );
      uint32_t returnValuePtr = spvModule.newVar( returnTypePtr, spv::StorageClassPrivate );
      spvModule.opStore( returnValuePtr, fogCtx.IsPixel ? fogCtx.oColor : spvModule.constf32( 0.0f ) );

      // Actually do the fog now we have all the vars in-place.

      spvModule.opSelectionMerge( skipFog, spv::SelectionControlMaskNone );
      spvModule.opBranchConditional( fogEnabled, doFog, skipFog );

      spvModule.opLabel( doFog );

      uint32_t wIndex = 3;
      uint32_t zIndex = 2;

      uint32_t w = spvModule.opCompositeExtract( floatType, fogCtx.vPos, 1, &wIndex );
      uint32_t z = spvModule.opCompositeExtract( floatType, fogCtx.vPos, 1, &zIndex );

      uint32_t depth = 0;
      if ( fogCtx.IsPixel )
          depth = spvModule.opFMul( floatType, z, spvModule.opFDiv( floatType, spvModule.constf32( 1.0f ), w ) );
      else {
          if ( fogCtx.RangeFog ) {
              std::array<uint32_t, 3> indices = { 0, 1, 2 };
              uint32_t pos3 = spvModule.opVectorShuffle( vec3Type, fogCtx.vPos, fogCtx.vPos, indices.size(), indices.data() );
              depth = spvModule.opLength( floatType, pos3 );
          } else
              depth = fogCtx.HasFogInput
              ? fogCtx.vFog
              : spvModule.opFAbs( floatType, z );
      }
      uint32_t fogFactor;
      if ( !fogCtx.IsPixel && fogCtx.IsFixedFunction && fogCtx.IsPositionT ) {
          fogFactor = fogCtx.HasSpecular
              ? spvModule.opCompositeExtract( floatType, fogCtx.Specular, 1, &wIndex )
              : spvModule.constf32( 1.0f );
      } else {
          uint32_t applyFogFactor = spvModule.allocateId();

          std::array<SpirvPhiLabel, 4> fogVariables;

          std::array<SpirvSwitchCaseLabel, 4> fogCaseLabels = { {
            { uint32_t( D3DFOG_NONE ),      spvModule.allocateId() },
            { uint32_t( D3DFOG_EXP ),       spvModule.allocateId() },
            { uint32_t( D3DFOG_EXP2 ),      spvModule.allocateId() },
            { uint32_t( D3DFOG_LINEAR ),    spvModule.allocateId() },
          } };

          spvModule.opSelectionMerge( applyFogFactor, spv::SelectionControlMaskNone );
          spvModule.opSwitch( fogMode,
                              fogCaseLabels[D3DFOG_NONE].labelId,
                              fogCaseLabels.size(),
                              fogCaseLabels.data() );

          for ( uint32_t i = 0; i < fogCaseLabels.size(); i++ ) {
              spvModule.opLabel( fogCaseLabels[i].labelId );

              fogVariables[i].labelId = fogCaseLabels[i].labelId;
              fogVariables[i].varId = [&] {
                  auto mode = D3DFOGMODE( fogCaseLabels[i].literal );
                  switch ( mode ) {
                  default:
                      // vFog
                  case D3DFOG_NONE:
                  {
                      if ( fogCtx.IsPixel )
                          return fogCtx.vFog;

                      if ( fogCtx.IsFixedFunction && fogCtx.HasSpecular )
                          return spvModule.opCompositeExtract( floatType, fogCtx.Specular, 1, &wIndex );

                      return spvModule.constf32( 1.0f );
                  }

                  // (end - d) / (end - start)
                  case D3DFOG_LINEAR:
                  {
                      uint32_t fogFactor = spvModule.opFSub( floatType, fogEnd, depth );
                      fogFactor = spvModule.opFMul( floatType, fogFactor, fogScale );
                      fogFactor = spvModule.opNClamp( floatType, fogFactor, spvModule.constf32( 0.0f ), spvModule.constf32( 1.0f ) );
                      return fogFactor;
                  }

                  // 1 / (e^[d * density])^2
                  case D3DFOG_EXP2:
                      // 1 / (e^[d * density])
                  case D3DFOG_EXP:
                  {
                      uint32_t fogFactor = spvModule.opFMul( floatType, depth, fogDensity );

                      if ( mode == D3DFOG_EXP2 )
                          fogFactor = spvModule.opFMul( floatType, fogFactor, fogFactor );

                      // Provides the rcp.
                      fogFactor = spvModule.opFNegate( floatType, fogFactor );
                      fogFactor = spvModule.opExp( floatType, fogFactor );
                      return fogFactor;
                  }
                  }
              }( );

              spvModule.opBranch( applyFogFactor );
          }

          spvModule.opLabel( applyFogFactor );

          fogFactor = spvModule.opPhi( floatType,
                                       fogVariables.size(),
                                       fogVariables.data() );
      }

      uint32_t fogRetValue = 0;

      // Return the new color if we are doing this in PS
      // or just the fog factor for oFog in VS
      if ( fogCtx.IsPixel ) {
          std::array<uint32_t, 4> indices = { 0, 1, 2, 6 };

          uint32_t color = fogCtx.oColor;

          uint32_t color3 = spvModule.opVectorShuffle( vec3Type, color, color, 3, indices.data() );

          std::array<uint32_t, 3> fogFacIndices = { fogFactor, fogFactor, fogFactor };
          uint32_t fogFact3 = spvModule.opCompositeConstruct( vec3Type, fogFacIndices.size(), fogFacIndices.data() );

          uint32_t lerpedFrog = spvModule.opFMix( vec3Type, fogColor, color3, fogFact3 );

          fogRetValue = spvModule.opVectorShuffle( vec4Type, lerpedFrog, color, indices.size(), indices.data() );
      } else
          fogRetValue = fogFactor;

      spvModule.opStore( returnValuePtr, fogRetValue );

      spvModule.opBranch( skipFog );

      spvModule.opLabel( skipFog );

      return spvModule.opLoad( returnType, returnValuePtr );
  }

  void DxsoCompiler::emitFog() {
    DxsoRegister color0;
    color0.id = DxsoRegisterId{ DxsoRegisterType::ColorOut, 0 };
    auto oColor0Ptr = this->emitGetOperandPtr(color0);

    DxsoRegister vFog;
    vFog.id = DxsoRegisterId{ DxsoRegisterType::RasterizerOut, RasterOutFog };
    auto vFogPtr = this->emitGetOperandPtr(vFog);

    DxsoRegister vPos;
    vPos.id = DxsoRegisterId{ DxsoRegisterType::MiscType, DxsoMiscTypeIndices::MiscTypePosition };
    auto vPosPtr = this->emitGetOperandPtr(vPos);

    D3D9FogContext fogCtx;
    fogCtx.IsPixel     = true;
    fogCtx.RangeFog    = false;
    fogCtx.RenderState = m_rsBlock;
    fogCtx.vPos        = m_module.opLoad(getVectorTypeId(vPosPtr.type),    vPosPtr.id);
    fogCtx.vFog        = m_module.opLoad(getVectorTypeId(vFogPtr.type),    vFogPtr.id);
    fogCtx.oColor      = m_module.opLoad(getVectorTypeId(oColor0Ptr.type), oColor0Ptr.id);
    fogCtx.IsFixedFunction = false;
    fogCtx.IsPositionT = false;
    fogCtx.HasSpecular = false;
    fogCtx.Specular    = 0;
    fogCtx.SpecUBO     = m_specUbo;

    m_module.opStore(oColor0Ptr.id, _DoFixedFunctionFog(m_spec, m_module, fogCtx));
  }

  void _DoFixedFunctionAlphaTest(SpirvModule& spvModule, const D3D9AlphaTestContext& ctx) {
    // Labels for the alpha test
    std::array<SpirvSwitchCaseLabel, 8> atestCaseLabels = {{
      { uint32_t(VK_COMPARE_OP_NEVER),            spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_LESS),             spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_EQUAL),            spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_LESS_OR_EQUAL),    spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_GREATER),          spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_NOT_EQUAL),        spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_GREATER_OR_EQUAL), spvModule.allocateId() },
      { uint32_t(VK_COMPARE_OP_ALWAYS),           spvModule.allocateId() },
    }};

    uint32_t atestBeginLabel   = spvModule.allocateId();
    uint32_t atestTestLabel    = spvModule.allocateId();
    uint32_t atestDiscardLabel = spvModule.allocateId();
    uint32_t atestKeepLabel    = spvModule.allocateId();
    uint32_t atestSkipLabel    = spvModule.allocateId();

    // if (alpha_func != ALWAYS) { ... }
    uint32_t boolType = spvModule.defBoolType();
    uint32_t isNotAlways = spvModule.opINotEqual(boolType, ctx.alphaFuncId, spvModule.constu32(VK_COMPARE_OP_ALWAYS));
    spvModule.opSelectionMerge(atestSkipLabel, spv::SelectionControlMaskNone);
    spvModule.opBranchConditional(isNotAlways, atestBeginLabel, atestSkipLabel);
    spvModule.opLabel(atestBeginLabel);

    // The lower 8 bits of the alpha ref contain the actual reference value
    // from the API, the upper bits store the accuracy bit count minus 8.
    // So if we want 12 bits of accuracy (i.e. 0-4095), that value will be 4.
    uint32_t uintType = spvModule.defIntType(32, 0);

    // Check if the given bit precision is supported
    uint32_t precisionIntLabel = spvModule.allocateId();
    uint32_t precisionFloatLabel = spvModule.allocateId();
    uint32_t precisionEndLabel = spvModule.allocateId();

    uint32_t useIntPrecision = spvModule.opULessThanEqual(boolType,
      ctx.alphaPrecisionId, spvModule.constu32(8));

    spvModule.opSelectionMerge(precisionEndLabel, spv::SelectionControlMaskNone);
    spvModule.opBranchConditional(useIntPrecision, precisionIntLabel, precisionFloatLabel);
    spvModule.opLabel(precisionIntLabel);

    // Adjust alpha ref to the given range
    uint32_t alphaRefIdInt = spvModule.opBitwiseOr(uintType,
      spvModule.opShiftLeftLogical(uintType, ctx.alphaRefId, ctx.alphaPrecisionId),
      spvModule.opShiftRightLogical(uintType, ctx.alphaRefId,
        spvModule.opISub(uintType, spvModule.constu32(8), ctx.alphaPrecisionId)));

    // Convert alpha ref to float since we'll do the comparison based on that
    uint32_t floatType = spvModule.defFloatType(32);
    alphaRefIdInt = spvModule.opConvertUtoF(floatType, alphaRefIdInt);

    // Adjust alpha to the given range and round
    uint32_t alphaFactorId = spvModule.opISub(uintType,
      spvModule.opShiftLeftLogical(uintType, spvModule.constu32(256), ctx.alphaPrecisionId),
      spvModule.constu32(1));
    alphaFactorId = spvModule.opConvertUtoF(floatType, alphaFactorId);

    uint32_t alphaIdInt = spvModule.opRoundEven(floatType,
      spvModule.opFMul(floatType, ctx.alphaId, alphaFactorId));

    spvModule.opBranch(precisionEndLabel);
    spvModule.opLabel(precisionFloatLabel);

    // If we're not using integer precision, normalize the alpha ref
    uint32_t alphaRefIdFloat = spvModule.opFDiv(floatType,
      spvModule.opConvertUtoF(floatType, ctx.alphaRefId),
      spvModule.constf32(255.0f));

    spvModule.opBranch(precisionEndLabel);
    spvModule.opLabel(precisionEndLabel);

    std::array<SpirvPhiLabel, 2> alphaRefLabels = {
      SpirvPhiLabel { alphaRefIdInt,    precisionIntLabel   },
      SpirvPhiLabel { alphaRefIdFloat,  precisionFloatLabel },
    };

    uint32_t alphaRefId = spvModule.opPhi(floatType,
      alphaRefLabels.size(),
      alphaRefLabels.data());

    std::array<SpirvPhiLabel, 2> alphaIdLabels = {
      SpirvPhiLabel { alphaIdInt,  precisionIntLabel   },
      SpirvPhiLabel { ctx.alphaId, precisionFloatLabel },
    };

    uint32_t alphaId = spvModule.opPhi(floatType,
      alphaIdLabels.size(),
      alphaIdLabels.data());

    // switch (alpha_func) { ... }
    spvModule.opSelectionMerge(atestTestLabel, spv::SelectionControlMaskNone);
    spvModule.opSwitch(ctx.alphaFuncId,
      atestCaseLabels[uint32_t(VK_COMPARE_OP_ALWAYS)].labelId,
      atestCaseLabels.size(),
      atestCaseLabels.data());

    std::array<SpirvPhiLabel, 8> atestVariables;

    for (uint32_t i = 0; i < atestCaseLabels.size(); i++) {
      spvModule.opLabel(atestCaseLabels[i].labelId);

      atestVariables[i].labelId = atestCaseLabels[i].labelId;
      atestVariables[i].varId   = [&] {
        switch (VkCompareOp(atestCaseLabels[i].literal)) {
          case VK_COMPARE_OP_NEVER:            return spvModule.constBool(false);
          case VK_COMPARE_OP_LESS:             return spvModule.opFOrdLessThan        (boolType, alphaId, alphaRefId);
          case VK_COMPARE_OP_EQUAL:            return spvModule.opFOrdEqual           (boolType, alphaId, alphaRefId);
          case VK_COMPARE_OP_LESS_OR_EQUAL:    return spvModule.opFOrdLessThanEqual   (boolType, alphaId, alphaRefId);
          case VK_COMPARE_OP_GREATER:          return spvModule.opFOrdGreaterThan     (boolType, alphaId, alphaRefId);
          case VK_COMPARE_OP_NOT_EQUAL:        return spvModule.opFUnordNotEqual      (boolType, alphaId, alphaRefId);
          case VK_COMPARE_OP_GREATER_OR_EQUAL: return spvModule.opFOrdGreaterThanEqual(boolType, alphaId, alphaRefId);
          default:
          case VK_COMPARE_OP_ALWAYS:           return spvModule.constBool(true);
        }
      }();

      spvModule.opBranch(atestTestLabel);
    }

    // end switch
    spvModule.opLabel(atestTestLabel);

    uint32_t atestResult = spvModule.opPhi(boolType,
      atestVariables.size(),
      atestVariables.data());
    uint32_t atestDiscard = spvModule.opLogicalNot(boolType, atestResult);

    // if (do_discard) { ... }
    spvModule.opSelectionMerge(atestKeepLabel, spv::SelectionControlMaskNone);
    spvModule.opBranchConditional(atestDiscard, atestDiscardLabel, atestKeepLabel);

    spvModule.opLabel(atestDiscardLabel);
    spvModule.opDemoteToHelperInvocation();
    spvModule.opBranch(atestKeepLabel);

    // end if (do_discard)
    spvModule.opLabel(atestKeepLabel);
    spvModule.opBranch(atestSkipLabel);

    // end if (alpha_test)
    spvModule.opLabel(atestSkipLabel);
  }
  
  void DxsoCompiler::emitPsProcessing() {
    uint32_t floatType = m_module.defFloatType(32);
    uint32_t uintType  = m_module.defIntType(32, 0);
    uint32_t uintPtr   = m_module.defPointerType(uintType, spv::StorageClassPushConstant);
    
    // Implement alpha test and fog
    DxsoRegister color0;
    color0.id = DxsoRegisterId{ DxsoRegisterType::ColorOut, 0 };
    auto oC0 = this->emitGetOperandPtr(color0);
    
    if (oC0.id) {
      if (m_programInfo.majorVersion() < 3)
        emitFog();

      uint32_t alphaRefMember = m_module.constu32(uint32_t(D3D9RenderStateItem::AlphaRef));
      uint32_t alphaComponentId = 3;

      D3D9AlphaTestContext alphaTestContext;
      alphaTestContext.alphaFuncId = m_spec.get(m_module, m_specUbo, SpecAlphaCompareOp);
      alphaTestContext.alphaPrecisionId = m_spec.get(m_module, m_specUbo, SpecAlphaPrecisionBits);
      alphaTestContext.alphaRefId = m_module.opLoad(uintType,
        m_module.opAccessChain(uintPtr, m_rsBlock, 1, &alphaRefMember));
      alphaTestContext.alphaId = m_module.opCompositeExtract(floatType,
        m_module.opLoad(m_module.defVectorType(floatType, 4), oC0.id),
        1, &alphaComponentId);

      _DoFixedFunctionAlphaTest(m_module, alphaTestContext);
    }
  }

  void DxsoCompiler::emitOutputDepthClamp() {
    // HACK: Some drivers do not clamp FragDepth to [minDepth..maxDepth]
    // before writing to the depth attachment, but we do not have acccess
    // to those. Clamp to [0..1] instead.

    if (m_ps.oDepth.id != 0) {
      auto result = emitValueLoad(m_ps.oDepth);

      result = emitSaturate(result);

      m_module.opStore(
        m_ps.oDepth.id,
        result.id);
    }
}


  void DxsoCompiler::emitVsFinalize() {
    this->emitMainFunctionBegin();

    this->emitInputSetup();
    m_module.opFunctionCall(
      m_module.defVoidType(),
      m_vs.functionId, 0, nullptr);
    this->emitLinkerOutputSetup();

    this->emitVsClipping();

    this->emitFunctionEnd();
  }

  void DxsoCompiler::emitPsFinalize() {
    this->emitMainFunctionBegin();

    this->emitInputSetup();

    bool canUsePixelFog = m_programInfo.majorVersion() < 3;

    if (canUsePixelFog) {
      // Look up vPos so it gets initted.
      DxsoRegister vPos;
      vPos.id = DxsoRegisterId{ DxsoRegisterType::MiscType, DxsoMiscTypeIndices::MiscTypePosition };
      this->emitGetOperandPtr(vPos);
    }

    if (m_ps.vPos.id != 0) {
      DxsoRegisterPointer fragCoord = this->emitRegisterPtr(
        "ps_frag_coord", DxsoScalarType::Float32, 4, 0,
        spv::StorageClassInput, spv::BuiltInFragCoord);

      DxsoRegisterValue val = this->emitValueLoad(fragCoord);
      val.id = m_module.opFSub(
        getVectorTypeId(val.type), val.id,
        m_module.constvec4f32(0.5f, 0.5f, 0.0f, 0.0f));

      m_module.opStore(m_ps.vPos.id, val.id);
    }

    if (m_ps.vFace.id != 0) {
      DxsoRegisterPointer faceBool = this->emitRegisterPtr(
        "ps_is_front_face", DxsoScalarType::Bool, 1, 0,
        spv::StorageClassInput, spv::BuiltInFrontFacing);

      DxsoRegisterValue frontFace = emitValueLoad(faceBool);
      DxsoRegisterValue selectOp = emitRegisterExtend(frontFace, 4);

      m_module.opStore(
        m_ps.vFace.id,
        m_module.opSelect(getVectorTypeId(m_ps.vFace.type), selectOp.id,
          m_module.constvec4f32( 1.0f,  1.0f,  1.0f,  1.0f),
          m_module.constvec4f32(-1.0f, -1.0f, -1.0f, -1.0f)));
    }

    m_module.opFunctionCall(
      m_module.defVoidType(),
      m_ps.functionId, 0, nullptr);

    // r0 in PS1 is the colour output register. Move r0 -> cO0 here.
    if (m_programInfo.majorVersion() == 1
    && m_programInfo.type() == DxsoProgramTypes::PixelShader) {
      DxsoRegister r0;
      r0.id = { DxsoRegisterType::Temp, 0 };

      DxsoRegister c0;
      c0.id = { DxsoRegisterType::ColorOut, 0 };

      DxsoRegisterValue val   = emitRegisterLoadRaw(r0, nullptr);
      DxsoRegisterPointer out = emitGetOperandPtr(c0);
      m_module.opStore(out.id, val.id);
    }

    // No need to setup output here as it's non-indexable
    // everything has already gone to the right place!

    this->emitPsProcessing();
    this->emitOutputDepthClamp();
    this->emitFunctionEnd();
  }



  uint32_t DxsoCompiler::getScalarTypeId(DxsoScalarType type) {
    switch (type) {
      case DxsoScalarType::Uint32:  return m_module.defIntType(32, 0);
      case DxsoScalarType::Sint32:  return m_module.defIntType(32, 1);
      case DxsoScalarType::Float32: return m_module.defFloatType(32);
      case DxsoScalarType::Bool:    return m_module.defBoolType();
    }

    throw DxvkError("DxsoCompiler: Invalid scalar type");
  }


  uint32_t DxsoCompiler::getVectorTypeId(const DxsoVectorType& type) {
    uint32_t typeId = this->getScalarTypeId(type.ctype);

    if (type.ccount > 1)
      typeId = m_module.defVectorType(typeId, type.ccount);

    return typeId;
  }


  uint32_t DxsoCompiler::getArrayTypeId(const DxsoArrayType& type) {
    DxsoVectorType vtype;
    vtype.ctype  = type.ctype;
    vtype.ccount = type.ccount;

    uint32_t typeId = this->getVectorTypeId(vtype);

    if (type.alength > 1) {
      typeId = m_module.defArrayType(typeId,
        m_module.constu32(type.alength));
    }

    return typeId;
  }


  uint32_t DxsoCompiler::getPointerTypeId(const DxsoRegisterInfo& type) {
    return m_module.defPointerType(
      this->getArrayTypeId(type.type),
      type.sclass);
  }

}
