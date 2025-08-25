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

    inline void setEnabled( bool bState ) 
    {
        bEnabled = bState;
    }

    inline void setName(const char* pName)
    {
        name = pName;
    }

    inline const char* getName() const 
    {
        return name.c_str();
    }

    inline uint32_t getObjectType() const
    {
        return objectType;
    }

protected:
    uint32_t objectType;
    std::string name;
    uint8_t bEnabled : 1;
};
