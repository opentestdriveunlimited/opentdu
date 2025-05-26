#pragma once

#include "game/gs_base.h"

class StreamingManager;

class GSWorld : public GameSystem {
public:
    const char* getName() const override { return "Service : World"; }

public:
    GSWorld();
    ~GSWorld();

    bool initialize( TestDriveGameInstance* pGameInstance ) override;
    void tick() override;
    void terminate() override;

private:
    StreamingManager* pStreamingManager;

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