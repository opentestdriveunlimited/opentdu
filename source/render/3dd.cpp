#include "shared.h"
#include "3dd.h"

static constexpr uint32_t kMaterialBankArrayMagic   = 0x414e424d; // MBNA (Material BaNk Array)
static constexpr uint32_t kHiearchyArrayMagic       = 0x41454948; // HIEA (HIEarchy Array)
static constexpr uint32_t kObjectArrayMagic         = 0x414a424f; // OBJA (OBJect Array)
static constexpr uint32_t kInstanceArrayMagic       = 0x41534e49; // INSA (INStance Array)

Render3DD::Render3DD()
     : pMatBankArray( nullptr )
     , pObjectArray( nullptr )
     , pInstanceArray( nullptr )
     , pHiearchyArray( nullptr )
{

}

Render3DD::~Render3DD()
{

}

bool Render3DD::parseSection(RenderFile::Section* pSection)
{
    switch (pSection->Type) {
    case kMaterialBankArrayMagic:
        pMatBankArray = pSection;
        return true;

    case kHiearchyArrayMagic:
        pHiearchyArray = pSection;
        return true;

    case kObjectArrayMagic:
        pObjectArray = pSection;
        return true;
        
    case kInstanceArrayMagic:
        pInstanceArray = pSection;
        return true;

    default:
        return true;
    };
}