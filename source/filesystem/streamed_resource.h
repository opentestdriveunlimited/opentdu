#pragma once

class StreamedResource {
public:
    StreamedResource()
    : bRegistered( false )
    {
        
    }

protected:
    uint8_t bRegistered : 1;
};
