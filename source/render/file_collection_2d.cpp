#include "shared.h"
#include "file_collection_2d.h"

void FileCollection2D::register2DB( Render2DB* render2DB )
{
    Render2DBs.insert( render2DB );
}

void FileCollection2D::register2DM( Render2DM* render2DM )
{
    Render2DMs.insert( render2DM );

    bindBitmapReferences( render2DM );
    bindUVAReferences( render2DM );
}

void FileCollection2D::bindBitmapReferences( Render2DM* render2DM )
{
    for ( const Render2DB* render2DB : Render2DBs ) {
        render2DM->bindBitmapReference( render2DB );
    }
}

void FileCollection2D::bindUVAReferences( Render2DM* render2DM )
{
    for ( const RenderUVA* renderUVA : UVAs ) {
        render2DM->bindUVAnimationReference( renderUVA );
    }
}

void FileCollection2D::unregister2DM( Render2DM* render2DM )
{
    Render2DMs.erase( render2DM );
}
