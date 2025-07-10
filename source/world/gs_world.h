#pragma once

#include "game/gs_base.h"
#include "render/file_collection_2d.h"

class StreamingManager;
class Render2DM;

class GSWorld : public GameSystem {
public:
    const char* getName() const override { return "Service : World"; }

public:
    GSWorld();
    ~GSWorld();

    bool initialize( TestDriveGameInstance* pGameInstance ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void pushGlobal2DM( Render2DM* pRender2DM );

private:
    StreamingManager* pStreamingManager;
    
    FileCollection2D collection2D;

    Eigen::Vector4f* pStreamPosition;
    Eigen::Vector4f staticStreamPosition;
    Eigen::Matrix4f previousTransformMatrix;
    Eigen::Vector4f camera[3];

    uint8_t bEnabled : 1;
    uint8_t bForceUpdate : 1;
    uint8_t bDirtyStreaming : 1;
    uint8_t bAreaStreamingEnabled : 1;
    uint8_t bIsCameraDirty : 1;
};

extern GSWorld* gpWorld;