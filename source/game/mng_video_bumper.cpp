#include "shared.h"
#include "mng_video_bumper.h"

MngVideoBumper::MngVideoBumper(GMVideoBumper* param_1)
    : MngFlash()
    , pParent( param_1 )
{
    bNeedDatabaseInit = true;
}

MngVideoBumper::~MngVideoBumper()
{

}

bool MngVideoBumper::initialize( TestDriveGameInstance* )
{
    return false;
}

void MngVideoBumper::tick( float, float )
{

}

void MngVideoBumper::terminate()
{

}
