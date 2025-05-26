#include "shared.h"
#include "flash_list.h"

#include "flash_player.h"

FlashList::FlashList()
    : pMovie( nullptr )
    , movieName( "" )
    , listSize( 0 )
    , numColumns( 0 )
    , selectedLineIndex( 0 )
    , selectedColumnIndex( 0 )
    , firstLine( 0 )
    , bInverted( false )
    , bOpened( false )
    , bIsEmpty( true )
    , timePressed( 0.0f )
    , prevInputAnimTime( 0.0f )
    , nextInputAnimTime( 0.0f )
{
}

FlashList::~FlashList()
{
}

void FlashList::setLineState(eLineState state, const int32_t index)
{
    int value;
    
    std::string local_80 = movieName;
    local_80 += "/line";
    local_80 += std::to_string( index + 1 );

    std::string local_100 = local_80;
    local_100 += "/:IsValid";
    
    const char* pFrameName = getLineStateFrameName(state);
    uint32_t validity = getLineStateValidity(state);

    pMovie->setVariableValue(local_100.c_str(), validity);
    pMovie->showFrame(pFrameName, local_80.c_str(), false);
}

const char *FlashList::getLineStateFrameName(eLineState state)
{
    switch(state) {
        case eLineState::FLLS_Normal: return "normal";
        case eLineState::FLLS_Empty: return "empty";
        case eLineState::FLLS_Red: return "red";
        case eLineState::FLLS_Blue: return "blue";
        case eLineState::FLLS_Gold: return "gold";
        case eLineState::FLLS_Disabled: return "disabled";
        case eLineState::FLLS_Green: return "green";
        case eLineState::FLLS_SubMenu: return "ss_menu";
        case eLineState::FLLS_NextStep: return "nextstep";
        case eLineState::FLLS_Selected: return "selected";
    }

    return "";
}

uint32_t FlashList::getLineStateValidity(eLineState state)
{
    switch(state) {
        case eLineState::FLLS_Normal: return 1;
        case eLineState::FLLS_Empty: return 0;
        case eLineState::FLLS_Red: return 0;
        case eLineState::FLLS_Blue: return 1;
        case eLineState::FLLS_Gold: return 0;
        case eLineState::FLLS_Disabled: return 0;
        case eLineState::FLLS_Green: return 0;
        case eLineState::FLLS_SubMenu: return 0;
        case eLineState::FLLS_NextStep: return 0;
        case eLineState::FLLS_Selected: return 1;
    }

    return 0;
}
