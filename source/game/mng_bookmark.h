#pragma once

#include "game/mng_base.h"

#include <mutex>

class MngBookmark : public Manager {
public:
    const char* getName() const override { return "Bookmark"; }

public:
    MngBookmark();
    ~MngBookmark();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;

private:
    std::mutex bookPicMutex;
};

extern MngBookmark* gpMngBookmark;
