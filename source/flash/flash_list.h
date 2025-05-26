#pragma once

#include "flash_entity.h"

struct FlashPlayer;

class FlashList : public FlashEntity
{
public:
    enum class eLineState {
        FLLS_Normal = 0,
        FLLS_Empty = 1,
        FLLS_Red = 2,
        FLLS_Blue = 3,
        FLLS_Gold = 4,
        FLLS_Disabled = 6,
        FLLS_Green = 7,
        FLLS_SubMenu = 8,
        FLLS_NextStep = 9,
        FLLS_Selected = 10
    };

public:
    FlashList();
    ~FlashList();

    void setLineState(eLineState state, const int32_t index);

protected:
    FlashPlayer* pMovie;
    std::string movieName;

    int32_t listSize;
    int32_t numColumns;
    int32_t selectedLineIndex;
    int32_t selectedColumnIndex;
    int32_t firstLine;

    uint8_t bInverted : 1;
    uint8_t bOpened : 1;
    uint8_t bIsEmpty : 1;

    float timePressed;
    float prevInputAnimTime;
    float nextInputAnimTime;

protected:
    const char* getLineStateFrameName(eLineState state);
    uint32_t getLineStateValidity(eLineState state);
};
