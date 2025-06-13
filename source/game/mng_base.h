#pragma once

class TestDriveGameInstance;

class Manager {
public:
    Manager();
    virtual ~Manager() {}

    virtual const char* getName() const { return "NoName"; }
    virtual void tick( float, float ) {}
    virtual bool initialize( TestDriveGameInstance* ) { return true; }
    bool initializeAsync( TestDriveGameInstance* );
    virtual void reset() {}
    virtual void draw() {}
    virtual void pause() {}
    virtual void terminate() {}

protected:
    Manager* pPreviousManager;
    Manager* pNextManager;
};
