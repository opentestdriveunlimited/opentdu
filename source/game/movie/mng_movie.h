#pragma once

#include "game/mng_base.h"

class MngMovie : public Manager {

public:
    const char* getName() const override { return "Movie"; }

public:
	MngMovie();
	~MngMovie();

	bool initialize( TestDriveGameInstance* ) override;
	void tick( float, float ) override;
	void terminate() override;
};
