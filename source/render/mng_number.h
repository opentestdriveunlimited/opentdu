#pragma once

#include "game/mng_base.h"
#include <Eigen/Dense>
#include "core/color.h"

class MngNumber : public Manager {
public:
    const char* getName() const override { return "Number"; }

public:
    MngNumber();
    ~MngNumber();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float ) override;
    virtual void draw() override;
    virtual void terminate()  override;

    void addNumber( Eigen::Vector3f& position, const uint32_t size, const ColorRGBA& color, int32_t number );

private:
    struct DrawCommand {
        Eigen::Vector3f Position;
        uint32_t        Size;
        ColorRGBA       Color;
        int32_t         Number;
    };

private:
    class DrawList*             pDrawList;
    std::vector<DrawCommand>    drawCommands;
    uint8_t                     bEnabled : 1;
};

extern MngNumber* gpMngNumber;
