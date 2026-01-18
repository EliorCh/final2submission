#pragma once
#include "GameBase.h"
#include "Utils.h"
#include "Steps.h"
#include "Results.h"
#include <vector>

class KeyboardGame : public GameBase {
private:
    bool saveMode;
    Screen fixedScreens[NUM_SCREENS];  // Constant screens like menu\instructions

protected:
    void handleInput() override;
    void pauseGame();
    void onGameEnd() override;
    void onPlayerDeath() override;
    int getDelay() const override { return KEYBOARD_DELAY; }
    bool getRiddleAnswer(Riddle* riddle, bool& outSolved) override;

public:
    explicit KeyboardGame(bool save = false);
    ~KeyboardGame();  

    void showMenu();
    void showInstructions();

};
