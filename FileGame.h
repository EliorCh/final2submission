#pragma once
#include "GameBase.h"
#include "Steps.h"
#include "Results.h"

constexpr int FALSE_SILENT_DELAY=10;

class FileGame : public GameBase {
private:
    bool silentMode; // true in -load -silent mode
    Results* expectedResults = nullptr;  // expected results loaded from .results file
    bool testPassed = false;     
    std::vector<std::string> failures;   // descriptions of test mismatches

    void compareResults();
    void printTestSummary() const;
    bool loadStepsFromFile(const std::string& filename);
    bool loadResultsFromFile(const std::string& filename);
    bool validateScreensHeader(std::ifstream& file);

protected:
    void handleInput() override;     // Reads input from steps file instead of keyboard
    void render() override;  
    void onGameEnd() override;
    void onPlayerDeath() override;
    void showMessage(const std::string& msg) override;
    int getDelay() const override {  // Controls game speed (no delay in silent mode)
        return silentMode ? 0 : LOAD_DELAY;
    }

    bool getRiddleAnswer(Riddle* riddle, bool& outSolved) override;
public:
    explicit FileGame(bool silent);    
    ~FileGame();

    bool loadFileGameResources();
};