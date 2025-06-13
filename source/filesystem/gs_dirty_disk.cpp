#include "shared.h"
#include "gs_dirty_disk.h"

#include "core/thread.h"

GSDirtyDisk* gpDirtyDisk = nullptr;

GSDirtyDisk::GSDirtyDisk()
    : GameSystem()
    , pThread( new TestDriveThread() )
    , bFailureReported( false )
{
    gpDirtyDisk = this;
}

GSDirtyDisk::~GSDirtyDisk()
{
    delete pThread;
}

void GSDirtyDisk::onDataLoadFailure()
{
    if (bFailureReported) {
        return;
    }

    OTDU_UNIMPLEMENTED;

    // TestDrive::DisplayModalPrompt
    //       (false,0x355fc5a,0,0,0,&this->field1_0x1c,0,0,"_Connection lost",true,false,
    //        MPT_Warning,gGSFlash.pFlashMessageBox);

    bFailureReported = true;
}
