#include "shared.h"
#include "mng_number.h"

#include "draw_list.h"
#include "render_scene.h"

#include "render/gs_render.h"

MngNumber::MngNumber()
    : Manager()
    , pDrawList( new DrawList() )
    , bEnabled( true )
{
    drawCommands.reserve( 16 );
}

MngNumber::~MngNumber()
{
    drawCommands.clear();
    delete pDrawList;
}

bool MngNumber::initialize(TestDriveGameInstance *)
{
    return pDrawList->initialize(100,1000, 0, true, nullptr, "MngNumber", false);
}

void MngNumber::tick(float, float)
{
    pDrawList->reset();
    drawCommands.clear();
}

void MngNumber::draw()
{
    if (!bEnabled) {
        return;
    }

    gpRender->getRenderPassByIndex( 0x4a ).pScene->enqueueDynamicDrawList( pDrawList );
}

void MngNumber::terminate()
{
    pDrawList->destroy();
}

void MngNumber::addNumber(Eigen::Vector3f &position, const uint32_t size, const ColorRGBA &color, int32_t number)
{
    DrawCommand drawCommand;
    drawCommand.Position = position;
    drawCommand.Size = size;
    drawCommand.Color = color;
    drawCommand.Number = number;

    drawCommands.push_back( drawCommand );
}
