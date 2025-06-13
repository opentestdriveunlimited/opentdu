#pragma once

class FlashPlayer;

class ModalMessageBox {
public:
    enum class eInputState {
        MMBIS_None = 0,
        MMBIS_Displayed1Sec	= 1,
        MMBIS_Displayed3Sec	= 2,
        MMBIS_Displayed	= 3,
        MMBIS_CloseRequested = 4,
        MMBIS_Closing = 5	
    };

    enum class eDisplayState {
        MMBDS_Closed = 0,
        MMBDS_Opening = 1,	
        MMBDS_Opened = 2,	
        MMBDS_Closing = 3	
    };

    using UserCallback_t = std::function<void(void*)>;

public:
    ModalMessageBox();
    ~ModalMessageBox();

    bool display(uint64_t hashcode, float param_2, int32_t param_5, UserCallback_t& pCallback, void *pCallbackData, bool param_7);

private:
    FlashPlayer* pFlashPlayer;
    eInputState inputState;
    eDisplayState displayState;

    UserCallback_t pUserCallback;

    float displayTime;
    float closeAfterDuration;

    uint8_t bUnknownFlag : 1;
    uint8_t bGameMessageBox : 1;

private:
    const char* getFrameToDisplay(const float param_1) const;
    void updateInputState(const float param_1);
};

extern ModalMessageBox* gpFlashMessageBox;
