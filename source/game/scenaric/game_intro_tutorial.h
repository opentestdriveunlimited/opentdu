#pragma once

class GameIntroTutorial {
public:
    GameIntroTutorial();
    ~GameIntroTutorial();

    bool isActive() const;

private:
    int32_t tutorialStep;
};
