#pragma once

#include <set>

#include "2dm.h"
#include "2db.h"
#include "uva.h"

struct FileCollection2D
{
    std::set<Render2DM*> Render2DMs;
    std::set<Render2DB*> Render2DBs;
    std::set<RenderUVA*> UVAs;

    void register2DM( Render2DM* render2DM );
    void unregister2DM( Render2DM* render2DM );

private:
    void bindBitmapReferences( Render2DM* render2DM );
    void bindUVAReferences( Render2DM* render2DM );
};
