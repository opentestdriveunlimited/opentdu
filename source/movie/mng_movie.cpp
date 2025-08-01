#include "shared.h"
#include "mng_movie.h"

MngMovie* gpMngMovie = nullptr;

MngMovie::MngMovie()
    : Manager()
    , render2DM{ Render2DM(), Render2DM() }
    , drawList()
{
    for (uint32_t i = 0; i < kNumEnqueuedFrames; i++ ) {
        render2DBUnk[i] = Render2DB();
        hashcodeUnk[i] = 0ull;
    }

    gpMngMovie = this;
}

MngMovie::~MngMovie()
{
    gpMngMovie = nullptr;
}

bool MngMovie::initialize( TestDriveGameInstance* pGameInstance )
{
    DrawStreams local_8;
    local_8.Normals = '\0';
    local_8.Specular = '\0';
    local_8.Tangent = '\0';
    local_8.Binormal = '\0';
    local_8.UVMap = '\x01';
    local_8.Index = '\x01';
    local_8.Diffuse = '\x01';

    drawList.initialize( 1, 2, 8, false, &local_8, "Movie", true );

    return false;
}

void MngMovie::tick( float, float )
{

}

void MngMovie::terminate()
{
}
