#pragma once

enum class ePopupAnswer {
    PA_Accept  = 0,
    PA_Cancel  = 1,
    PA_Unknown = 2
};

class PopupCallback {
public:
    PopupCallback() {}
    virtual ~PopupCallback() {}

protected:
    virtual void onClose( enum ePopupAnswer answer, void* param_2, int32_t param_3 ) = 0;
};
