#include "shared.h"
#include "modal_message_box.h"

#include "gs_flash.h"

ModalMessageBox::ModalMessageBox()
    : pFlashPlayer( nullptr )
    , inputState( eInputState::MMBIS_None )
    , displayState( eDisplayState::MMBDS_Closed )
    , displayTime( 0.0f )
    , closeAfterDuration( 0.0f )
    , bGameMessageBox( false )
{
}

ModalMessageBox::~ModalMessageBox()
{

}

bool ModalMessageBox::display(uint64_t hashcode, float waitDuration, int32_t param_5, UserCallback_t& pCallback, void *pCallbackData, bool param_7)
{
    if (inputState != eInputState::MMBIS_None) {
        return false;
    }

    waitDuration = 0.0f;
    pUserCallback = std::bind(pCallback, pCallbackData);
    OTDU_UNIMPLEMENTED; // TODO:
    return false;
}
