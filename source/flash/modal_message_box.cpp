#include "shared.h"
#include "modal_message_box.h"

#include "gs_flash.h"
#include "database/gs_database.h"

ModalMessageBox* gpFlashMessageBox = nullptr;

ModalMessageBox::ModalMessageBox()
    : pFlashPlayer( nullptr )
    , inputState( eInputState::MMBIS_None )
    , displayState( eDisplayState::MMBDS_Closed )
    , displayTime( 0.0f )
    , closeAfterDuration( 0.0f )
    , bUnknownFlag( false )
    , bGameMessageBox( false )
{
    gpFlashMessageBox = this;
}

ModalMessageBox::~ModalMessageBox()
{

}

bool ModalMessageBox::display(uint64_t hashcode, float param_2, int32_t param_5, UserCallback_t* pCallback, void *pCallbackData, bool param_7)
{
    if (inputState != eInputState::MMBIS_None) {
        return false;
    }

    displayTime = 0.0f;
    if (pCallback != nullptr)
    {
        pUserCallback = std::bind(*pCallback, pCallbackData);
    }
    else
    {
        pUserCallback = nullptr;
    }
    bUnknownFlag = param_7;
    if (param_7) {
        int32_t iVar1 = gpFlash->getNumMovies(1);
        for (int32_t i = 0; i < iVar1; i++) {
            MoviePlayer* pMVar2 = gpFlash->getMoviePlayer(1, i);
            if (pMVar2 != nullptr) {
                pMVar2->bEnabled = true;
            }
        }
        OTDU_UNIMPLEMENTED;
        // FUN_0099b3f0("CONFIGPC",&gGSFlash,0,false,4); TODO: Not sure
    }

    FlashPlayer* pPlayer = gpFlash->getFlashPlayer("GENERAL");
    const char* pFrameToDisplay = getFrameToDisplay(param_2);

    if (pPlayer != nullptr) {
        const char* pcVar4 = gpDatabase->getStringByHashcode(param_5, hashcode);
        pPlayer->setVariableValue("/:msg", pcVar4);
        pPlayer->showFrame( "/waiting", pFrameToDisplay, true);
    }

    updateInputState(param_2);
    return true;
}

ModalMessageBox::eDisplayState ModalMessageBox::getDisplayState() const
{
    return displayState;
}

ModalMessageBox::eInputState ModalMessageBox::getInputState() const
{
    return inputState;
}

const char *ModalMessageBox::getFrameToDisplay(const float param_1) const
{
    if (param_1 <= 1.0)         return "waiting_1";
    else if (param_1 <= 3.0)    return "waiting_3";
    else                        return "waiting_more";
}

void ModalMessageBox::updateInputState(const float param_1)
{
    if (param_1 <= 1.0)         inputState = eInputState::MMBIS_Displayed1Sec;
    else if (param_1 <= 3.0)    inputState = eInputState::MMBIS_Displayed3Sec;
    else                        inputState = eInputState::MMBIS_Displayed;
}
