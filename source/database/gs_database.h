#pragma once

#include "game/gs_base.h"

#include "db_achievement.h"
#include "db_aftermarket_pack.h"
#include "db_bot.h"
#include "db_brand.h"
#include "db_car_color.h"
#include "db_car_pack.h"
#include "db_car_rim.h"
#include "db_car_shop.h"
#include "db_car_physics_data.h"
#include "db_clothe.h"
#include "db_hair.h"
#include "db_house.h"
#include "db_interior.h"
#include "db_menu.h"
#include "db_pnj.h"
#include "db_rim.h"
#include "db_subtitle.h"
#include "db_tutorial.h"

class GSDatabase : public GameSystem {
public:
    int32_t NbChallengeNames;
    int32_t NbChallengeDesc;

public:
    const char* getName() const override { return "DataBase"; }

public:
    GSDatabase();
    ~GSDatabase();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

private:
    uint8_t bInitialized : 1;
};

extern GSDatabase* gpDatabase;
