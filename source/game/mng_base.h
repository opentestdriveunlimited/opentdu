#pragma once

class TestDriveGameInstance;

class Manager {
public:
    Manager();
    virtual ~Manager() {}

    virtual const char* getName() const { return "NoName"; }
    virtual void tick( float, float ) = 0;
    virtual bool initialize( TestDriveGameInstance* ) = 0;
    bool initializeAsync( TestDriveGameInstance* );
    virtual void reset() = 0;
    virtual void draw() = 0;
    virtual void pause() = 0;
    virtual void terminate() = 0;

protected:
    Manager* pPreviousManager;
    Manager* pNextManager;
};
