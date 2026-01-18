#include "FileGame.h"

FileGame::FileGame(bool silent) : GameBase(), silentMode(silent) { }  

FileGame::~FileGame() = default; 


bool FileGame::loadFileGameResources()
{
    setGame();
    // Initialize core game state (players, counters, flags)
    initGame();

    // Load + validate screen files and riddles
    if (!loadGameFiles()) {
        return false;
    }

    // Load + validate recorded gameplay steps 
    if (!loadStepsFromFile("adv-world.steps")) {
        return false;
    }

    // Load + validate recorded gameplay steps 
    if (!loadResultsFromFile("adv-world.results")) {
        return false;
    }

    if (silentMode) {
        // create container for actual results for comparing later
        setResults(new Results());
    }

    return true;
}

bool FileGame::loadStepsFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        showError("Failed to open steps file.");
        return false;
    }
    
    // Validate screen files listed in header
    if (!validateScreensHeader(file)) {
        return false;
    }
    
    // Read steps from file
    Steps* loadedSteps = Steps::loadSteps(file);
    if (!loadedSteps) {
        showError("Failed to parse steps.");
        return false;
    }

    setSteps(loadedSteps);
    return true;
}

bool FileGame::loadResultsFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        showError("Failed to open results file.");
        return false;
    }
    
    // Validate screen files listed in header
    if (!validateScreensHeader(file)) {
        return false;
    }
    
    // Read expected results (stream is positioned after header)
    expectedResults = Results::loadResults(file);
    if (!expectedResults) {
        showError("Failed to read results.");
        return false;
    }

    return true;
}

bool FileGame::validateScreensHeader(std::ifstream& file){
    std::string line;
    std::vector<std::string> screensFromFile;
    
    // Expect header line: "# screens"
    if (!std::getline(file, line)) {
        showError("Screen header not found.");
        return false;
    }
    
    if (line != "# screens") {
        showError("Screen header not found.");
        return false;
    }
    
    // Read screen file names (one per line)
    while (std::getline(file, line)) {        
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            break; 
        }
        
        screensFromFile.push_back(line);
    }
    auto loadedScreens = getScreenSourceFiles();
    
    if (screensFromFile != loadedScreens) {
        showError("Screen files listed in file do not match loaded screens.");
        return false;
    }
    
    return true;
}

void FileGame::handleInput() {
    if (getSteps()->isEmpty()) {  // No more recorded steps available

        // Game already ended normally - exit run loop
        if (gameOver) {
            isRunning = false;
            return;
        }
        // If one player finished, waiting for the other - continue
        if (isGameInFinalPhase()) {
            return;   
        }

        static int emptyStepsCount = 0;
        emptyStepsCount++;

        if (emptyStepsCount < 5) {
            // Wait a bit more - game might end soon
            return;
        }
        // Steps ended before reaching a valid game end
        showError("ERROR: Steps ended before game finished.");
        isRunning = false;
        return;
    }
    // Execute step only when its iteration matches current game cycle
    if (getSteps()->isNextStepOnIteration(gameCycles)) {
        char ch = getSteps()->popStep();
        processKey(ch);
    }
}

void FileGame::render() {   // Suppress rendering in silent mode
    if (!silentMode) GameBase::render();
}

void FileGame::onPlayerDeath()
{
    showMessage("Player is dead. Better luck next time... -_-");
    gameOver = true;
    isRunning = false;
}

void FileGame::onGameEnd() {
    // In silent load mode, validate results and print test outcome
    if (silentMode) {
        compareResults();
        printTestSummary();
    }
}

void FileGame::compareResults() {
    // Assume success until a mismatch is found
    testPassed = true;
    failures.clear();

    // Get expected (from file) and actual (from game run) results
    const auto& expected = expectedResults->getResults();
    const auto& actual = getResults()->getResults();

    auto itExp = expected.begin();
    auto itAct = actual.begin();

    // Compare results one by one, in chronological order
    while (itExp != expected.end() && itAct != actual.end())
    {
        // Check that the event occurred at the same game cycle
        if (itExp->first != itAct->first) {
            testPassed = false;
            failures.push_back(
                "Iteration mismatch: expected " +
                std::to_string(itExp->first) +
                ", got " +
                std::to_string(itAct->first)
            );
            return;
        }

        // Check that the logical result is the same
        if (!(itExp->second == itAct->second)) {
            testPassed = false;
            failures.push_back(
                "Result mismatch at iteration " +
                std::to_string(itExp->first)
            );
            return;
        }

        ++itExp;
        ++itAct;
    }

    // Check for extra or missing results
    if (itExp != expected.end() || itAct != actual.end()) {
        testPassed = false;
        failures.push_back("Number of results mismatch");
    }

}

void FileGame::printTestSummary() const{  
    // Test passed: print summary and exit
    if (testPassed) {
        std::cout << "TEST PASSED" << std::endl;
        return;
    }
    // Test failed: print all detected mismatches
    std::cout << "TEST FAILED" << std::endl;

    for (const auto& msg : failures) {
        std::cout << "- " << msg << std::endl;
    }
}

bool FileGame::getRiddleAnswer(Riddle* riddle, bool& outSolved)
{
    // Get expected riddle answer for this iteration
    std::string expA;

    if (!expectedResults->getRiddleAtIteration(gameCycles, expA)) {
        return false;  // No riddle found in results file at this iteration
    }
    // Compare file answer to riddle's correct answer
    outSolved = matchRiddleAnswer(riddle->getAnswer(), expA);
    if (silentMode) {
        // Record actual result
        getResults()->addRiddleRes(gameCycles, riddle->getQuestion(), expA, outSolved);
    }
    return true;
}

void FileGame::showMessage(const std::string& msg) {
    // In FILE mode: show message but don't wait for input
    Utils::clearScreen();

    // Display the message centered
    Utils::gotoxy(0, 10);
    std::cout << msg << std::endl;
    std::cout << std::flush;

    Utils::delay(2000);  // 2 seconds
}