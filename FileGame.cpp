#include "FileGame.h"

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

    if (silentMode) {
        if (!loadResultsFromFile("adv-world.results")) {
            return false;
        }
        // create container for actual results
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
    // seed for riddles shuffle
    unsigned int savedSeed;
    if (!(file >> savedSeed)) {
        showError("Failed to read random seed from steps file.");
        return false;
    }
    setGameSeed(savedSeed);

    std::string dummy;
    std::getline(file, dummy);

    if (!validateScreensHeader(file)) {
        showError("Steps file does not match current level files.");
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

    /*
    // Validate screen files listed in header
    if (!validateScreensHeader(file)) {
        return false;
    }
    */
    // Read expected results (stream is positioned after header)
    expectedResults = Results::loadResults(file);
    if (!expectedResults) {
        showError("Failed to read results.");
        return false;
    }

    return true;
}

bool FileGame::validateScreensHeader(std::ifstream& file) const
{
    std::string line;
    std::vector<std::string> screensFromFile;

    // Expect header line: "screens"
    if (!std::getline(file, line) || line != "# screens") {
        showError("Invalid steps file format: missing '# screens' header.");
        return false;
    }

    std::streampos currentPos = file.tellg();

    // Read screen file names (one per line)
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '#' && line != "# screens") {
            file.seekg(currentPos);
            break;
        }
        if (!line.empty()) screensFromFile.push_back(line);
        currentPos = file.tellg();
    }

    // Validate that every screen listed in the file exists in the loaded screens
    if (screensFromFile != getScreenSourceFiles()) {
        showError("Screen files listed in file do not match loaded screens.");
        return false;
    }
    return true;
}

void FileGame::handleInput() {
    if (getSteps()->isEmpty()) {    // No more steps -> game loop ends ->
        isRunning = false;    // -> show test results
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
        failures.emplace_back("Number of results mismatch");
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
    std::string expQ, expA;
    bool expOk;

    // Get expected riddle result for this iteration
    if (!expectedResults->getRiddleAtIteration(gameCycles, expQ, expA, expOk))
        return false;

    // Verify this is the same riddle
    if (expQ != riddle->getQuestion())
        return false;

    // Compare file answer to riddle's correct answer
    outSolved = Utils::toUpperCase(expA) == Utils::toUpperCase(riddle->getAnswer());

    // Validate logical correctness
    if (outSolved != expOk)
        return false;

    // Record actual result
    getResults()->addRiddleRes(gameCycles, expQ, expA, outSolved);

    return true;
}