#pragma once

class TestDriveGameInstance;

class GameSystem {
public:
    GameSystem();
    virtual ~GameSystem() {}

    virtual const char* getName() const { return "NoName"; }
    virtual bool initialize( TestDriveGameInstance* ) = 0;
    virtual void tick() {};
    virtual void draw( float deltaTime ) {};
    virtual void terminate() = 0;
    virtual void pause() {};
    virtual void reset();

protected:
    TestDriveGameInstance*  pGameInstance;
    uint32_t                index;
    uint8_t                 bPaused : 1;
};
