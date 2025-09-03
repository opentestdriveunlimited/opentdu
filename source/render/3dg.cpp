#include "shared.h"
#include "3dg.h"

#include "render/gs_render_helper.h"
#include "render/gs_render.h"

static constexpr uint32_t kGeometryArrayMagic   =  0x414f4547; // GEOA (GEOmetry Array)

static uint32_t gNumPrimitives = 0u; // DAT_00fe88a0

Render3DG::Render3DG()
     : RenderFile()
     , pGeometryArray( nullptr )
{

}

Render3DG::~Render3DG()
{

}

bool Render3DG::parseSection(RenderFile::Section *pSection)
{
    // FUN_0050a2a0
    switch (pSection->Type) {
    case kGeometryArrayMagic:
        pGeometryArray = pSection;
        return true;

    case kPrimitiveMagic:
        return parsePrimitiveSection(pSection);

    default:
        return true;
    };
}

void Render3DG::unparseSection(RenderFile::Section *pSection)
{
    // FUN_0050a2d0
    if (pSection->Type == kPrimitiveMagic) {
        unstreamPrimitive(pSection);
    }
}

bool Render3DG::parsePrimitiveSection(RenderFile::Section *pSection)
{
    // FUN_00512280
    if (pSection == nullptr) {
        return false;
    }

    Primitive* pPrimitive = (Primitive*)pSection;
    pPrimitive->bUploaded = false;

    // FUN_00512040(0xfad0d0,(RenderList *)(param_1 + 1));

    return true;
}

void Render3DG::unstreamPrimitive(RenderFile::Section *pSection)
{
    // FUN_005122a0
    Primitive* param_1 = (Primitive*)(pSection + 1);
    OTDU_UNIMPLEMENTED; //FUN_005120a0((astruct_3 *)&UNK_00face70.field_0x260,peVar1);
    if (param_1->Type == ePrimitiveType::PT_HMap) {
        unstreamHeightmap(param_1);
    } else {
        unstreamGeometry(param_1);
    }
}

void Render3DG::unstreamHeightmap(Primitive *param_1)
{
    // FUN_005ff010
    if (!param_1->bUploaded) {
        return;
    }
    param_1->bUploaded = false;
    OTDU_UNIMPLEMENTED; 
    // GPUBuffer** piVar2 = FUN_005fed10(param_1);
    // if (*piVar2 != nullptr) {
    //     gpRender->getRenderDevice()->destroyBuffer(*piVar2);
    // }
    // *piVar2 = nullptr;
}

void Render3DG::unstreamGeometry(Primitive *param_1)
{
    // FUN_005fe670
}
