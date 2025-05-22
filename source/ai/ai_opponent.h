#pragma once

class AIOpponent {
public:
    inline bool isActive() const { return bActive; }
    inline bool isMaster() const { return bMaster; }

public:
    AIOpponent();
    ~AIOpponent();

private:
    uint8_t bActive : 1;
    uint8_t bMaster : 1;
};