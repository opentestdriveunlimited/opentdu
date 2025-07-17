#include "shared.h"
#include "gs_cloth_system.h"

GSClothSystem* gpClothSystem = nullptr;

GSClothSystem::GSClothSystem()
    : GameSystem()
{
    gpClothSystem = this;
}

GSClothSystem::~GSClothSystem()
{

}

bool GSClothSystem::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSClothSystem::terminate()
{

}
