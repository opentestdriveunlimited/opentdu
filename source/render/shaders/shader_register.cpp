#include "shared.h"
#include "shader_register.h"

#include "shader_table_master.h"

#include "render/material.h"

ShaderRegister gShaderRegister = {};

ShaderRegister::ShaderRegister()
{

}

void ShaderRegister::retrieveShadersForMaterial( Material* param_1, const uint32_t index )
{
    OTDU_ASSERT( param_1 );

    if ( param_1->pVertexShaders[index] != nullptr ) {
        param_1->pVertexShaders[index] = nullptr;
    }

    if ( param_1->pPixelShaders[index] != nullptr ) {
        param_1->pPixelShaders[index] = nullptr;
    }

    MaterialParameterFlags local_58;
    uint64_t local_8 = 0x3dafe4f6b9bae7f4;
    if ( param_1->NumParams != 0 ) {
        fillParameterFlags( param_1, &local_58 );
        if ( local_8 == 0xcaca336d83078865 ) {
            local_58.NumPrelight = '\x01';
        }
    }

    OTDU_UNIMPLEMENTED;
}

void ShaderRegister::fillParameterFlags( Material* param_1, MaterialParameterFlags* param_2 )
{
    OTDU_UNIMPLEMENTED;

    if ( param_1->NumParams != 0 ) {
        MaterialParameter* pParameters = ( MaterialParameter* )( (int8_t*)param_1 + 0xd0 );
        if ( pParameters->Type == 1 ) {

        } else if ( pParameters->Type == 2 ) {

         }
    }
}
