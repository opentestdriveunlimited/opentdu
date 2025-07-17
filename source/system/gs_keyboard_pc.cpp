#include "shared.h"
#include "gs_keyboard_pc.h"

GSKeyboardPC* gpKeyboardPC = nullptr;

GSKeyboardPC::GSKeyboardPC()
    : GameSystem()
{
    gpKeyboardPC = this;
}

GSKeyboardPC::~GSKeyboardPC()
{

}

bool GSKeyboardPC::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSKeyboardPC::terminate()
{

}
