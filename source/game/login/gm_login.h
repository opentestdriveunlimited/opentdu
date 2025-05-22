#pragma once

#include "game/gm_base.h"
#include "core/popup_callback.h"

class GMLogin : public GameMode, public PopupCallback {
public:
    GMLogin();
    ~GMLogin();
};
