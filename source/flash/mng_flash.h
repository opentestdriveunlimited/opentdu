#pragma once

#include "game/mng_base.h"
#include "flash_entity.h"

struct FlashWidget {
    FlashEntity* pWidgetInstance = nullptr;

    union {
        uint32_t WidgetID = 0u;
        char     WidgetName[4];
    };
};

class MngFlash : public Manager, public FlashEntity {
public:
    const char* getName() const override { return "Flash"; }

public:
    MngFlash();
    ~MngFlash();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;

    void addWidget(const char pName[4], FlashEntity* pWidget);
    void removeWidgets();

protected:
    std::vector<FlashWidget> flashWidgets;

    uint8_t bInitialized : 1;
    uint8_t bWantAsyncInstall : 1;
    uint8_t bNeedDatabaseInit : 1;
};
