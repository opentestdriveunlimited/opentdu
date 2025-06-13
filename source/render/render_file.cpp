#include "shared.h"
#include "render_file.h"

RenderFile::RenderFile()
    : pHeader( nullptr )
    , pSections( nullptr )
{

}

RenderFile::~RenderFile()
{

}

bool RenderFile::parseSection( Section* pSection )
{
    pHeader = ( Header* )pSection;
    pSections = ( pSection + 1 );
    return true;
}

bool RenderFile::parseFile()
{
    return false;
}
