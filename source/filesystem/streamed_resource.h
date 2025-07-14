#pragma once

class TestDriveThread;

class StreamedResource {
public:
    StreamedResource()
    : bWaitingForInstanciation( true )
    {
        
    }

    virtual void instanciate()
    {
        OTDU_LOG_WARN("StreamedResource::instanciate() is unimplemented!!\n");
        OTDU_ASSERT(false);
    }

    virtual void instanciate(TestDriveThread* param_1)
    {

    }

    inline void markAsInstanciated()
    {
        bWaitingForInstanciation = false;
    }

protected:
    uint8_t bWaitingForInstanciation : 1;
};
