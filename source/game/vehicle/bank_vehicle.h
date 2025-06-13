#pragma once

#include "filesystem/bank.h"
#include "filesystem/streamed_resource.h"

#include "render/uva.h"
#include "render/2db.h"
#include "render/3dg.h"

class PhysFactory;

class BankVehicle: public Bank, public StreamedResource
{
public:
    enum class eState {
        BVS_None = 0,
        BVS_Ready = 1,
        BVS_Loading = 2
    };

public:
    BankVehicle();
    ~BankVehicle();

private:
    template<typename T>
    struct ResourcesToLoad {
        T Resources;
        std::string ResourcePath;
    };

private:
    eState resourceState;
    std::string name;

    uint8_t bLoadAsync : 1;
    uint8_t bIsDefaultRes : 1;

    std::vector<ResourcesToLoad<RenderUVA>> setUVA;
    std::vector<ResourcesToLoad<Render2DB>> set2DB;

    std::string file3DGPath;
    Render3DG render3DG;

    void* pPhysicsBinary;
    void* pTorqueBinary;

    PhysFactory* pPhysFactory;
};
