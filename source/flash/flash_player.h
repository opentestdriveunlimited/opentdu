#pragma once

class FlashPlayer {
public:
    FlashPlayer();
    ~FlashPlayer();

    void showFrame(const char* param_1, const char* param_2, bool param_3);
    void setVariableValue(const char* param_1, const char* param_2);
    void setVariableValue(const char *param_1, const uint32_t param_2);

    int32_t getVariableAsInt(const char* param_1);
};
