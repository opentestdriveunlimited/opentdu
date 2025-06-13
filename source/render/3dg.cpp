#include "shared.h"
#include "3dg.h"

static constexpr uint32_t kGeometryArrayMagic   =  0x414f4547; // GEOA (GEOmetry Array)
static constexpr uint32_t kPrimitiveMagic       =  0x4d495250; // PRIM (PRIMitive)

Render3DG::Render3DG()
     : RenderFile()
     , pGeometryArray( nullptr )
{

}

Render3DG::~Render3DG()
{

}

bool Render3DG::parseSection(RenderFile::Section* pSection)
{
    switch (pSection->Type) {
    case kGeometryArrayMagic:
        pGeometryArray = pSection;
        return true;

    case kPrimitiveMagic:
        OTDU_UNIMPLEMENTED;
        // cVar1 = FUN_00512280(param_2);
        // if (cVar1 == '\0') {
        //     return 0;
        // }
        return true;

    default:
        return true;
    };
}