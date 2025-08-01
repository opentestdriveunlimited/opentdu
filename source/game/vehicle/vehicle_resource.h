#pragma once

class BankVehicle;

#include "render/2dm.h"
#include "render/3dd.h"
#include "render/file_collection_2d.h"
#include "render/file_collection_3d.h"

struct VehicleResource {
    BankVehicle* pResourceBank = nullptr;
    std::string Render2DMPath = "";
    Render2DM* pResource2DM = nullptr;
    std::string Render3DDPath = "";
    Render3DD* pResource3DD = nullptr;
    FileCollection2D Collection2D = {};
    FileCollection3D Collection3D = {};
};
