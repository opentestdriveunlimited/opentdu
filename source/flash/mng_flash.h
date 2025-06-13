#pragma once

#include "game/mng_base.h"
#include "flash_entity.h"

class FlashPlayer;

struct FlashWidget {
    FlashEntity* pWidgetInstance = nullptr;

    union {
        uint32_t WidgetID = 0u;
        char     WidgetName[4];
    };
};

struct FlashMessage
{
    union
    {
        char pAsChar[4];
        uint32_t AsInteger = 0u;
    };
    
    constexpr FlashMessage( const uint32_t magic )
        : AsInteger( magic )
    {
        
    }

    constexpr bool operator == ( const uint32_t magic ) const
    {
        return AsInteger == magic;
    }
    
    constexpr bool operator == ( const FlashMessage& other ) const
    {
        return AsInteger == other.AsInteger;
    }

    constexpr operator uint32_t () const
    {
        return AsInteger;
    }
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

    virtual bool onMessage(FlashMessage& pMessage, FlashPlayer* pPlayer) { return false; }

protected:
    std::vector<FlashWidget> flashWidgets;

    uint8_t bInitialized : 1;
    uint8_t bWantAsyncInstall : 1;
    uint8_t bNeedDatabaseInit : 1;
};
