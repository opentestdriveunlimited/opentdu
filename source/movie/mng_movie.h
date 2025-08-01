#pragma once

#include "game/mng_base.h"

#include "render/2db.h"
#include "render/2dm.h"
#include "render/draw_list.h"

class MngMovie : public Manager {
public:
    const char* getName() const override { return "Movie"; }

public:
	MngMovie();
	~MngMovie();

	bool initialize( TestDriveGameInstance* ) override;
	void tick( float, float ) override;
	void terminate() override;

private:
	static constexpr uint32_t kNumEnqueuedFrames = 8;

private:
	Render2DM render2DM[2];
	DrawList drawList;
	
	// TODO: Might be frames for Bink?
	Render2DB render2DBUnk[kNumEnqueuedFrames];
	uint64_t hashcodeUnk[kNumEnqueuedFrames];

	uint8_t bEnabled : 1;
	uint8_t bInitialized : 1;
	uint8_t bPlaying : 1;
	uint8_t bDone : 1;

	int32_t frameIndex;
};

extern MngMovie* gpMngMovie;
