#pragma once

class RenderObjectBase {
public:
    RenderObjectBase()
        : objectType( 0 )
        , name( "" )
        , bEnabled( true )
    {

    }

    virtual ~RenderObjectBase()
    {
        name.clear();
        bEnabled = false;
    }

protected:
    uint32_t objectType;
    std::string name;
    uint8_t bEnabled : 1;
};
