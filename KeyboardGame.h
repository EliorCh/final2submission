#pragma once
#include "GameBase.h"

class KeyboardGame : public GameBase {
private:
    static constexpr int NUM_FIXED_SCREENS = 2;  // Menu and Instructions
    bool saveMode;
    Screen fixedScreens[NUM_FIXED_SCREENS];      // Constant screens like menu\instructions

protected:
    void handleInput() override;
    void pauseGame();
    void onGameEnd() override;
    void onPlayerDeath() override;
    int getDelay() const override { return KEYBOARD_DELAY; }
    bool getRiddleAnswer(Riddle* riddle, bool& outSolved) override;

public:
    explicit KeyboardGame(bool save = false);
    ~KeyboardGame() override;

    void showMenu();
    void showInstructions() const;
};
